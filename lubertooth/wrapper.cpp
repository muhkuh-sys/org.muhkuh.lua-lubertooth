#include "wrapper.h"



/*--------------------------------------------------------------------------*/


const char *version_libubertooth(void)
{
	return libubertooth_get_version();
}


const char *release_libubertooth(void)
{
	return libubertooth_get_release();
}


const char *version_libbtbb(void)
{
	return btbb_get_version();
}


const char *release_libbtbb(void)
{
	return btbb_get_release();
}


/*--------------------------------------------------------------------------*/


Ubertooth::Ubertooth(void)
 : m_ptUt(NULL)
 , m_iIsConnected(0)
 , m_iDevice(-1)
{
	m_ptUt = ubertooth_init();
}


Ubertooth::~Ubertooth(void)
{
	if( m_ptUt!=NULL )
	{
		ubertooth_stop(m_ptUt);
		m_ptUt = NULL;
	}
}


int Ubertooth::connect(int iDevice)
{
	int iResult;


	if( m_ptUt==NULL )
	{
		fprintf(stderr, "No state available.\n");
		iResult = -1;
	}
	else if( m_iIsConnected!=0 )
	{
		fprintf(stderr, "Already connected. Disconnect first.\n");
		iResult = -1;
	}
	else
	{
		/* Try to connect to the Ubertooth device. */
		iResult = ubertooth_connect(m_ptUt, iDevice);
		if( iResult<0 )
		{
			fprintf(stderr, "Failed to connect to device %d.\n", iDevice);
			iResult = -1;
		}
		else
		{
			/* Compare the API versions of the PC library and the firmware. */
			iResult = ubertooth_check_api(m_ptUt);
			if( iResult<0 )
			{
				ubertooth_stop(m_ptUt);
				fprintf(stderr, "API mismatch.\n");
				iResult = -1;
			}
			else
			{
				/* Register a cleanup handler at exit. */
				register_cleanup_handler(m_ptUt, 0);

				/* The instance is now connected. */
				m_iIsConnected = 1;
				m_iDevice = iDevice;
				iResult = 0;
			}
		}
	}

	return iResult;
}


int Ubertooth::specan(SWIGLUA_REF tLuaFn)
{
	int iResult;
	int iLower = 2402;
	int iUpper= 2480;


	m_tLuaCallbackFn = tLuaFn;

	if( m_ptUt==NULL )
	{
		fprintf(stderr, "No state available.\n");
		iResult = -1;
	}
	else if( m_iIsConnected!=1 )
	{
		fprintf(stderr, "Not connected. Connect first.\n");
		iResult = -1;
	}
	else
	{
		/* Initialize USB transfer. */
		iResult = ubertooth_bulk_init(m_ptUt);
		if( iResult<0 )
		{
			fprintf(stderr, "Failed to initialize bulk communication.\n");
			iResult = -1;
		}
		else
		{
			/* Start the receive thread. */
			iResult = ubertooth_bulk_thread_start();
			if( iResult<0 )
			{
				fprintf(stderr, "Failed to start the receive thread.\n");
				iResult = -1;
			}
			else
			{
				/* Set the Ubertooth stick to "specan" mode and start to receive packets. */
				iResult = cmd_specan(m_ptUt->devh, iLower, iUpper);
				if( iResult<0 )
				{
					fprintf(stderr, "Failed to start SPECAN mode.\n");
					iResult = -1;
				}
				else
				{
					/* Receive and process each packet. */
					while( !m_ptUt->stop_ubertooth )
					{
						ubertooth_bulk_receive(m_ptUt, Ubertooth::s_callback_specan, this);
					}

					ubertooth_bulk_thread_stop();
				}
			}
		}
	}

	return iResult;
}


void Ubertooth::s_callback_specan(ubertooth_t *ptUt , void *pvUser)
{
	Ubertooth *ptThis;


	/* Get the pointer to the instance. */
	ptThis = (Ubertooth*)pvUser;
	if( ptThis!=NULL )
	{
		/* Continue with the instance. */
		ptThis->callback_specan(ptUt);
	}
}


void Ubertooth::callback_specan(ubertooth_t *ptUt)
{
	usb_pkt_rx tRx;
	int iCnt;
	uint16_t usFrequency;
	int8_t iRssi;
	uint32_t ulTime;
	lua_State *ptL;
	int iRef;
	int iOldTopOfStack;
	int iResult;
	const char *pcErrMsg;
	const char *pcErrDetails;
	int fStillRunning;
	int iLuaType;


	/* Get the next packet from the FIFO. */
	tRx = fifo_pop(ptUt->fifo);

	/* Get the reference to the callback function. */
	ptL = m_tLuaCallbackFn.L;
	iRef = m_tLuaCallbackFn.ref;
	if( ptL!=NULL && iRef!=LUA_NOREF && iRef!=LUA_REFNIL )
	{
		/* Get the current stack position. */
		iOldTopOfStack = lua_gettop(ptL);
		lua_rawgeti(ptL, LUA_REGISTRYINDEX, iRef);

		/* Push the time on the stack as the first argument. */
		lua_pushnumber(ptL, ((double)tRx.clk100ns)/10000000);

		/* Create a new table for the values.
		 * The table will have 0 array elements and DMA_SIZE/3 record elements.
		 */
		lua_createtable(ptL, 0, DMA_SIZE/3);
		for(iCnt=0; iCnt<DMA_SIZE-2; iCnt+=3)
		{
			/* Extract the frequency and level. */
			usFrequency = (tRx.data[iCnt] << 8) | tRx.data[iCnt+1];
			iRssi = (int8_t)tRx.data[iCnt+2];

			lua_pushnumber(ptL, usFrequency);
#if LUA_VERSION_NUM>=504
			lua_pushinteger(ptL, iRssi);
#else
			lua_pushnumber(ptL, iRssi);
#endif
			lua_settable(ptL, -3);
		}

		/* Call the function with 2 arguments and 1 result. */
		iResult = lua_pcall(ptL, 2, 1, 0);
		if( iResult!=0 )
		{
			switch( iResult )
			{
			case LUA_ERRRUN:
				pcErrMsg = "runtime error";
				break;
			case LUA_ERRMEM:
				pcErrMsg = "memory allocation error";
				break;
			default:
				pcErrMsg = "unknown errorcode";
				break;
			}
			pcErrDetails = lua_tostring(ptL, -1);
			fprintf(stderr, "callback function failed: %s (%d): %s", pcErrMsg, iResult, pcErrDetails);
			fStillRunning = 0;
		}
		else
		{
			/* Get the function's return value. */
			iLuaType = lua_type(ptL, -1);
			if( iLuaType!=LUA_TNUMBER && iLuaType!=LUA_TBOOLEAN )
			{
				fprintf(stderr, "callback function returned a non-boolean type: %d", iLuaType);
				fStillRunning = 0;
			}
			else
			{
				iResult = lua_toboolean(ptL, -1);
				fStillRunning = (iResult!=0);
			}
		}

		/* Return old stack top. */
		lua_settop(ptL, iOldTopOfStack);
	}
	else
	{
		/* No callback function -> stop immediately. */
		fStillRunning = 0;
	}

	if( fStillRunning==0 )
	{
		ptUt->stop_ubertooth = 1;
	}
}

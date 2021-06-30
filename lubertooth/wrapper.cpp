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


int Ubertooth::specan(void)
{
	int iResult;
	int iLower = 2402;
	int iUpper= 2480;


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
	usb_pkt_rx rx = fifo_pop(ptUt->fifo);
	int j;
	uint16_t frequency;
	int8_t rssi;


	/* Process each received block. */
	for (j = 0; j < DMA_SIZE-2; j += 3) {
		frequency = (rx.data[j] << 8) | rx.data[j + 1];
		rssi = (int8_t)rx.data[j + 2];

		printf("%f, %d, %d\n", ((double)rx.clk100ns)/10000000, frequency, rssi);
	}
}

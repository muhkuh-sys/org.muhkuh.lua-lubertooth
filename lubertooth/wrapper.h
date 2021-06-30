/* Both ubertooth and lua are C projects. */
#ifdef __cplusplus
extern "C" {
#endif
#include "ubertooth.h"
#include "lua.h"
#ifdef __cplusplus
}
#endif

/* ubertooth uses defines from stdint. The wrapper should use these defines too. */
#include <stdint.h>



#ifndef SWIGRUNTIME
#include <swigluarun.h>
#endif



#ifndef __WRAPPER_H__
#define __WRAPPER_H__

const char *version_libubertooth(void);
const char *release_libubertooth(void);
const char *version_libbtbb(void);
const char *release_libbtbb(void);


class Ubertooth
{
public:
	Ubertooth(void);
	~Ubertooth(void);

	int connect(int iDevice);
	int specan(void);

	/* Do not wrap the callback function. It is only for internal use. */
#ifndef SWIG
	static void s_callback_specan(ubertooth_t *ptUt , void *pvUser);
	void callback_specan(ubertooth_t *ptUt);
#endif

	/* Do not wrap the private members. They are private. */
#ifndef SWIG
private:
	ubertooth_t *m_ptUt;
	int m_iIsConnected;
	int m_iDevice;
#endif
};

#endif  /* __WRAPPER_H__ */

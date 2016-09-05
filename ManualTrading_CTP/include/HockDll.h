#ifndef	KEY_HOCK_H
#define KEY_HOCK_H

#undef KEY_HOCK_API
#ifdef _LIB_EXPORT
#define KEY_HOCK_API _declspec(dllexport)
#else
#define KEY_HOCK_API _declspec(dllimport)
#endif


class KeyHockSpi
{
public:

	virtual void OnKeyDown(int key) = 0;
};



class KEY_HOCK_API KeyDownHock
{ 

public:

	static KeyDownHock* CreateKeyHock();

	virtual bool Init() = 0;

	virtual void RegisterSpi(KeyHockSpi *KeySpi) = 0;

	virtual bool Release() = 0;
};

#endif
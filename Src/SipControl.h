#ifndef SIPCONTROL_H
#define SIPCONTROL_H

class SipControl {
	RECT rSipRect;
public:
	BOOL Init() { return TRUE; }

	BOOL GetSipStat(BOOL *pStatus);
	BOOL SetSipStat(BOOL bActive);
	RECT GetRect() { return rSipRect; }
};


#endif
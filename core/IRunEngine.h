#ifndef __IRUN_ENGINE_H__
#define __IRUN_ENGINE_H__

#pragma once

class IRunEngine {
public:
	virtual BOOL Initialize(HINSTANCE hInstance) = 0;
	virtual int Run() = 0;
	virtual void Release() = 0;
};

#endif
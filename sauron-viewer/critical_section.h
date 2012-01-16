#pragma once


class CSection : public CRITICAL_SECTION 
{
public:
    CSection() {
        ::InitializeCriticalSection ( this );
    }
    ~CSection() {
        ::DeleteCriticalSection ( this );
    }
    void Enter() {
        ::EnterCriticalSection ( this );
    }
    void Leave() {
        ::LeaveCriticalSection ( this );
    }
};

class Lock
{
public:
    Lock(CSection* cs) : m_cs(cs)
    {
        m_cs->Enter();
    }
    ~Lock()
    {
        m_cs->Leave();
    }
private:
    CSection* m_cs;
};

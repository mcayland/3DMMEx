/*
 * Platform-specific definitions
 */

#ifndef PLATFORM_H
#define PLATFORM_H

/****************************************
    Mutex (critical section) object
****************************************/
typedef class MUTX *PMUTX;

class MUTX
{
  protected:
    void *opaque;

  public:
    MUTX(void);
    ~MUTX(void);

    void Enter(void);
    void Leave(void);
};

/****************************************
    Current thread id
****************************************/

extern inline long LwThreadCur(void);

/***************************************************************************
    Universal scalable application clock and other time stuff
***************************************************************************/

extern const unsigned long kdtsSecond;
extern unsigned long TsCurrentSystem(void);
extern unsigned long DtsCaret(void);

#endif //! PLATFORM_H

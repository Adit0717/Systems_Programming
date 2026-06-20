code Synch

  -- OS Class: Project 2 Part 1
  --
  -- Aditya Krishnamurthi

-----------------------------  Semaphore  ---------------------------------

  behavior Semaphore
    -- This class provides the following methods:
    --    Up()  ...also known as "V" or "Signal"...
    --         Increment the semaphore count.  Wake up a thread if
    --         there are any waiting.  This operation always executes
    --         quickly and will not suspend the thread.
    --    Down()   ...also known as "P" or "Wait"...
    --         Decrement the semaphore count.  If the count would go
    --         negative, wait for some other thread to do an Up()
    --         first.  Conceptually, the count will never go negative.
    --    Init(initialCount)
    --         Each semaphore must be initialized.  Normally, you should
    --         invoke this method, providing an 'initialCount' of zero.
    --         If the semaphore is initialized with 0, then a Down()
    --         operation before any Up() will wait for the first
    --         Up().  If initialized with i, then it is as if i Up()
    --         operations have been performed already.
    --
    -- NOTE: The user should never look at a semaphore's count since the value
    -- retrieved may be out-of-date, due to other threads performing Up() or
    -- Down() operations since the retrieval of the count.

      ----------  Semaphore . Init  ----------

      method Init (initialCount: int)
          if initialCount < 0
            FatalError ("Semaphore created with initialCount < 0")
          endIf
          count = initialCount
          waitingThreads = new List [Thread]
        endMethod

      ----------  Semaphore . Up  ----------

      method Up ()
          var
            oldIntStat: int
            t: ptr to Thread
          oldIntStat = SetInterruptsTo (DISABLED)
          if count == 0x7fffffff
            FatalError ("Semaphore count overflowed during 'Up' operation")
          endIf
          count = count + 1
          if count <= 0
            t = waitingThreads.Remove ()
            t.status = READY
            readyList.AddToEnd (t)
          endIf
          oldIntStat = SetInterruptsTo (oldIntStat)
        endMethod

      ----------  Semaphore . Down  ----------

      method Down ()
          var
            oldIntStat: int
          oldIntStat = SetInterruptsTo (DISABLED)
          if count == 0x80000000
            FatalError ("Semaphore count underflowed during 'Down' operation")
          endIf
          count = count - 1
          if count < 0
            waitingThreads.AddToEnd (currentThread)
            currentThread.Sleep ()
          endIf
          oldIntStat = SetInterruptsTo (oldIntStat)
        endMethod

  endBehavior

-----------------------------  Mutex  ---------------------------------

  behavior Mutex
    -- This class provides the following methods:
    --    Lock()
    --         Acquire the mutex if free, otherwise wait until the mutex is
    --         free and then get it.
    --    Unlock()
    --         Release the mutex.  If other threads are waiting, then
    --         wake up the oldest one. (Do not give the lock to the one you wake up)
    --    Init()
    --         Each mutex must be initialized.
    --    IsHeldByCurrentThread()
    --         Return TRUE iff the current (invoking) thread holds a lock
    --         on the mutex.

      ----------  Mutex . Init  ----------

      method Init () --hint: look at the semaphore init() and keep what is neccessary for a Mutex
          -- FatalError ("Unimplemented method") --these calls should be commented when implementing method

          heldBy = null
          waitingThreads = new List [Thread]

        endMethod

        /* Init() - For my understanding.,
          Initialize the fields. 
          heldBy points to the thread that is currently holding the lock and while initializing this should be null.
          waitingThreads is of type List[Thread] and that is initially empty.
        */

      ----------  Mutex . Lock  ----------

      method Lock () 
          --FatalError ("Unimplemented method")
          --hints--
          --ensure atomic execution
          --you will need declare a new variable oldIntStat of type int. similar to how the semaphore is implemented (check code above for sempaphore implementation).
          --test if the current thread is already holding lock, if so call FatalError ("Attempting to lock a mutex by a thread already holding it") 
          --you will need to check the value of the variable heldBy (which is an instance variable declared in the header file)
          --intead of checking for count you want to check if lock is held by another thread. heldBy is == null if not held by another thread.
          --if not held by another thread, this thread will hold the lock
          --otherwise, add self to waiting threads list and then go to sleep
          --reset interrupt status.

          var
            oldIntStat: int

          oldIntStat = SetInterruptsTo (DISABLED)

          if self.IsHeldByCurrentThread ()
            FatalError ("Attempting to lock a mutex by a thread already holding it")
          endIf

          /* Caused Race condition
          if heldBy == null
            heldBy = currentThread
          else 
            waitingThreads.AddToEnd (currentThread)
            currentThread.Sleep ()
            heldBy = currentThread
          endIf
          */

          while heldBy != null
            waitingThreads.AddToEnd(currentThread)
            currentThread.Sleep()
          endWhile

          heldBy = currentThread

          oldIntStat = SetInterruptsTo (oldIntStat)
          
        endMethod

        /* Lock method - For my understanding.,
           Declare oldIntStat. 
           While acquiring lock, interrupts should not happen, so do "SetInterruptsTo(DISABLED)"
           
           While lock is not free, put yourself into the waiting list and go to sleep

           After acquiring the lock interrupts can happen, so do "oldIntStat = SetInterruptsTo(oldIntStat)"
           While disabling interrupts oldIntStat will preserve the value "Enabled" and at the end we use this preserved value to enable the interrupts.
        */

      ----------  Mutex . Unlock  ----------

      method Unlock ()
          --FatalError ("Unimplemented method")
          --hints--
          --ensure atomic execution
          --you will need declare a new variable oldIntStat of type int. similar to how the semaphore is implemented (check code above for sempaphore implementation).
          --test if the current thread is trying to unlock a lock it is not holding, if so call a FatalError with an appropriate message
          --you will need to check the value of the variable heldBy (which is an instance variable declared in the header file)
          --complete the nessarry steps.. note: waitingThreads.Remove() returns null if there is no one waiting. 
          --reset interrupt status.

          var
            oldIntStat: int
            nextTh: ptr to Thread

          oldIntStat = SetInterruptsTo (DISABLED)

          if ! self.IsHeldByCurrentThread ()
            FatalError ("Current Thread is trying to unlock when it does not own the lock")
          endIf

          nextTh = waitingThreads.Remove ()
          
          if nextTh != null
            nextTh.status = READY
            readyList.AddToEnd (nextTh)
          endIf

          heldBy = null

          oldIntStat = SetInterruptsTo (oldIntStat)
          
        endMethod

        /* Unlock method - For my understanding.,
           Disable interrupts for atomic execution.
           If the lock is not held by the current thread and it is trying to unlock then raise an error message.
           Remove the head from the waitingThreads and set it to nextTh.
           If there are no waiting threads then simply release the lock.
           Else -> Currently this nextTh would be in waiting list. Put it in the readyList and update the status to READY and then release the lock.
           Enable the interrupts.
        */

      ----------  Mutex . IsHeldByCurrentThread  ----------

      method IsHeldByCurrentThread () returns bool
        --FatalError ("Unimplemented method")

        if heldBy == currentThread
          return true
        endIf

        return false
      endMethod

  endBehavior

-----------------------------  Condition  ---------------------------------

  behavior Condition
    -- This class is used to implement monitors.  Each monitor will have a
    -- mutex lock and one or more condition variables.  The lock ensures that
    -- only one process at a time may execute code in the monitor.  Within the
    -- monitor code, a thread can execute Wait() and Signal() operations
    -- on the condition variables to make sure certain condions are met.
    --
    -- The condition variables here implement "Mesa-style" semantics, which
    -- means that in the time between a Signal() operation and the awakening
    -- and execution of the corrsponding waiting thread, other threads may
    -- have snuck in and run.  The waiting thread should always re-check the
    -- data to ensure that the condition which was signalled is still true.
    --
    -- This class provides the following methods:
    --    Wait(mutex)
    --         This method assumes the mutex has alreasy been locked.
    --         It unlocks it, and goes to sleep waiting for a signal on
    --         this condition.  When the signal is received, this method
    --         re-awakens, re-locks the mutex, and returns.
    --    Signal(mutex)
    --         If there are any threads waiting on this condition, this
    --         method will wake up the oldest and schedule it to run.
    --         However, since this thread holds the mutex and never unlocks
    --         it, the newly awakened thread will be forced to wait before
    --         it can re-acquire the mutex and resume execution.
    --    Broadcast(mutex)
    --         This method is like Signal() except that it wakes up all
    --         threads waiting on this condition, not just the next one.
    --    Init()
    --         Each condition must be initialized.

      ----------  Condition . Init  ----------

      method Init ()
          waitingThreads = new List [Thread]
        endMethod

      ----------  Condition . Wait  ----------

      method Wait (mutex: ptr to Mutex)
          var
            oldIntStat: int
          if ! mutex.IsHeldByCurrentThread ()
            FatalError ("Attempt to wait on condition when mutex is not held")
          endIf
          oldIntStat = SetInterruptsTo (DISABLED)
          mutex.Unlock ()
          waitingThreads.AddToEnd (currentThread)
          currentThread.Sleep ()
          mutex.Lock ()
          oldIntStat = SetInterruptsTo (oldIntStat)
        endMethod

      ----------  Condition . Signal  ----------

      method Signal (mutex: ptr to Mutex)
          var
            oldIntStat: int
            t: ptr to Thread
          if ! mutex.IsHeldByCurrentThread ()
            FatalError ("Attempt to signal a condition when mutex is not held")
          endIf
          oldIntStat = SetInterruptsTo (DISABLED)
          t = waitingThreads.Remove ()
          if t
            t.status = READY
            readyList.AddToEnd (t)
          endIf
          oldIntStat = SetInterruptsTo (oldIntStat)
        endMethod

      ----------  Condition . Broadcast  ----------

      method Broadcast (mutex: ptr to Mutex)
          var
            oldIntStat: int
            t: ptr to Thread
          if ! mutex.IsHeldByCurrentThread ()
            FatalError ("Attempt to braodcast a condition when lock is not held")
          endIf
          oldIntStat = SetInterruptsTo (DISABLED)
          while true
            t = waitingThreads.Remove ()
            if t == null
              break
            endIf
            t.status = READY
            readyList.AddToEnd (t)
          endWhile
          oldIntStat = SetInterruptsTo (oldIntStat)
        endMethod

  endBehavior

endCode

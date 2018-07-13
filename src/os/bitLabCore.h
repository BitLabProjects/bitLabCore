#ifndef _BITLABCORE_H_
#define _BITLABCORE_H_

class bitLabCore {
public:
  bitLabCore();

  void run();

protected:
  virtual void init() = 0;
  virtual void mainLoop() = 0;
};

#endif
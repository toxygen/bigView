#include <iostream>
#include <unistd.h>
#include <sys/times.h>
#include <bits/time.h>
#include "debug.h"

int
main()
{
  _VAR( CLOCKS_PER_SEC );
  _VAR( sysconf (2) );
}

#!/bin/bash
  # if mod_gui is NOT running then launch UI
  # if running, do not interrupt or launch it again
/bin/ps aux | grep -q "mod\_gui"
if [ $? -eq 1 ]; then
  isEVF=0
    # if EVF mode then force LCD
  st cap capdtm getusr MONITOROUT | grep -q EVF
  if [ $? -eq 0 ]; then
    isEVF=1
    st app disp lcd
    sleep 0.2
  fi
    # get in high priority mode
  /usr/bin/renice -n -10 --pid $$
    # finally run the user interface
  /opt/usr/apps/nx-remote-controller-mod/externals/mod_gui apps
  sync
    # return to EVF mode
  if [ $isEVF -eq 1 ]; then
    st app disp evf
  fi
fi


#!/bin/sh

timeout=$1
shift
text="$@"

yad --undecorated --no-focus --on-top --center --no-buttons --timeout=$timeout --timeout-indicator=bottom --text="$text"

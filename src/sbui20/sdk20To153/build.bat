

gcc -O2 -c RzSwitchbladeSDK2.c -o RzSwitchbladeSDK2.o -Wall -s
%DEVCDLLW% --implib libRzSwitchbladeSDK2.a RzSwitchbladeSDK2.o --def RzSwitchbladeSDK2.def -L%DEVCL% --export-all-symbols -o RzSwitchbladeSDK2.dll --output-def sdk2.def -mconsole -s


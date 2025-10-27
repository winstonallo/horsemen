 clear && rm -f ./a/ls && cp $(which ls) ./a && chmod +w ./a/ls && clear && make && ./Famine ; echo $? && ./a/ls ; echo $?

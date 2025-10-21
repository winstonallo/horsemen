 clear && rm -f ./a/ls && cp $(which ls) ./a && chmod +w ./a/ls && touch /tmp/infected && clear && make && ./Famine ; echo $? && rm /tmp/infected && ./a/ls ; echo $?

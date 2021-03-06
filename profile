#for zoe
export PLATFORM=/home/zoe/platform
export BUSINESS=/home/zoe/business

export PATH=$PATH:$PLATFORM/srv
export PATH=$PATH:$PLATFORM/ins

#for unixodbc
export PATH=$PATH:/usr/local/unixodbc/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/unixodbc/lib

#for openssl
export PATH=$PATH:/usr/local/openssl/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/openssl/lib

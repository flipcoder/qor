#BUGLE_CHAIN=trace LD_PRELOAD=libbugle.so ./qor
##BUGLE_CHAIN=trace LD_PRELOAD=libbugle.so ./qor

rm *.trace
rm *.dump
apitrace trace ./qor
apitrace dump qor.trace > qor.dump
#vim qor.dump


// Test that when no file is provided
// there is no crash/infinite loop.
@import "WarpBuf.chug"

WarpBuf s1 => dac;

while(true) {
	<<<"silent">>>;
	100::ms => now;
}
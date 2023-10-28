# Test code for Auriga

## Building

You can simply make it using the makefile:

```
make
```

## To execute

There is an `data_in.txt` example, you can run the binary:

```
./test_is
```
The application will read the `data_in.txt` and parse the message by
storing the message in a temporary structure.

Later, the message is encoded into binary, and manipulated according
to the requirements specified by Auriga.

At the end, the output is written in the `data_out.txt` file.

### Example of output

Here is the output example based on the provided `data_in.txt` file:

```
message type: 0x01
initial message length: 0x10
initial message data bytes: 0xfffffffffffffffffffffffc
initial CRC-32: 0xffa35348
modified message length: 0x10
modified message data bytes with mask: 0xfefefefefffffffffefefefc
modified CRC-32: 0x5fce94fb
```

# Test code for Auriga

## Building

To build, you can simply run the `build.sh` script.

```
./build.sh
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

# lib1541img

This is a library for creating and modifying images of disks for the Commodore
1541 disk drive (`D64`). It's written in plain ISO C (C11) with no dependencies
other than a standard C library for a "hosted" environment and should work on
almost any systems, as long as enough RAM is available (4 to 8MB should be by
far enough, depending on the size of the C library used).

It features an object-oriented API, modelling the disk and its contents in 3
layers:

* The `D64` class and related classes are the "physical" layer, modelling the
  disk itself with its tracks and sectors.
* The `CbmdosVfs` class and related classes form the "virtual filesystem" (VFS)
  modelling logical disk properties (name, id, etc) and the files contained on
  the disk.
* The `CbmdosFs` class acts as the "glue layer", responsible for the actual
  layout of a `CbmdosVfs` on a `D64`. In normal configuration, it "owns" a
  `D64` and a `CbmdosVfs`, listening to events from the VFS and updating the
  disk accordingly.

A simple event mechanism is included in the library, as well as a simple
logging interface for enabling diagnostic output from the library.

## Features

* Read and write D64 disk images
* Support for 40- and 42-track images
* Support for BAM formats of SpeedDOS, DolphinDOS and PrologicDOS
* Import and export C64 files either as raw content or in PC64 container
  format (.P00/.U00/...)
* Compress and extract 4-pack and 5-pack ZipCode
* ZipCode files on D64 disk images
* Create and import LyNX archive files

### Not supported

* raw GCR images (.g64)
* GEOS files
* any trackloading mechanisms used in many games and demos
* C128 boot sectors
* D64 error info

### Warnings

If you use the functions of this library on a disk that has any of the
unsupported features, you might lose data. If you aren't sure, better work
on a copy of your disk image file.

Especially, error information appended to a D64 is silently ignored and won't
be included when you save the image.

## Build and install

To get the latest version of the source code, you will need `git` and use the
following commands:

    git clone https://github.com/excess-c64/lib1541img
    cd lib1541img
    git submodule update --init

If you want to get a specific version, e.g. v0.9, use these commands:

    git clone https://github.com/excess-c64/lib1541img
    cd lib1541img
    git checkout v0.9
    git submodule update --init

To build and install, use these commands:

    make -j4 strip
    make install

This will install to `/usr/local`, so you will need root privileges for the
`make install` step. The build system understands a lot of standard variables
like `DESTDIR`, `prefix`, etc.

If you are on a system that doesn't use GNU make by default (like for example
FreeBSD), install a GNU make package and use the command `gmake` instead of
`make`.

## Documentation

To build html documentation, you will need *Doxygen* installed on your system.
Then use

    make html

This will create a full html documentation of the library's API.
The html documentation is also installed with `make install` if it has been
built before.

## General usage

*lib1541img* uses an object-oriented design. Methods of a class are prefixed
with the class name, for example `FileData_appendByte()` will append a single
byte to a `FileData` object.

The handle to an object is a pointer to it, as returned by a "constructor"
function. The default constructor for a class is called "create", e.g.
`D64_create()` is used for creating a new `D64` object. All other methods will
require you to pass the object pointer as the first argument. Every class
also has a "destructor" function called "destroy", for example 
`D64_destroy()`, which you must call to release memory used by the object.

Some methods take pointers to *other* objects. The object may or may not take
ownership of the other object, here's how you can know:

* If the parameter for the other object is declared as a `const` pointer,
  ownership is not taken over, so you still have to destroy the other object
  yourself.

  Example: `CbmdosFile_setName()` will set a file name from a `const char *`
  pointer to a string you provide, but you will still own the original string
  afterwards.

* If the parameter for the other object is declared as a non-`const` pointer,
  ownership *is* taken over, so you must not destroy the other object
  yourself.

  Example: `CbmdosFs_fromImage()` will create a new `CbmdosFs` from an
  existing `D64` image and take ownership of that image.

* The exception to this rule is a method that writes data *to*
  another object. In this case, the `self` pointer (the first parameter) will
  be `const`. Such a method won't take ownership of the other object.

  Example: `ZcFileSet_saveVfs()` will store the files of a Zipcode file set
  into an existing `CbmdosVfs`.

## Threading

*lib1541im* doesn't use threads, but can be used in a threaded program if some
care is taken:

* Events work simply by direct calls, `CbmdosVfs` subscribes Events from each
  `CbmdosFile` and `CbmdosFs` subscribes events from `CbmdosVfs`. So, if you
  use a `CbmdosFs` in one thread, make sure you use the attached `CbmdosVfs`
  and its files only in the same thread.
* A *logwriter* is set globally and will be shared by all threads. The default
  implementation does nothing, so it's inherently thread-safe.
  `setFileLogger()` sets a logwriter that appends to an opened file handle,
  which *should* be thread-safe on most platforms. If you provide your own
  logwriter with `setCustomLogger()` and use *lib1541img* functions from
  different threads, it's your responsibility to make your logwriter
  thread-safe.

## Static linking

If you want to link *lib1541img* statically to your program, make sure the
preprocessor macro `STATIC_1541IMG` is defined, otherwise you might get linking
errors. With a GCC-compatible compiler, you can pass the flag
`-DSTATIC_1541IMG`.

Note by default, the static library isn't built. To build and install the
static library, use these commands:

    make -j4 staticlibs
    make installstaticlibs

## Example

The following code will read `foo.d64`, add a file `bar.prg` to the disk and
save the result as `baz.d64`:

    #include <1541img/cbmdosfile.h>
    #include <1541img/cbmdosfs.h>
    #include <1541img/cbmdosfsoptions.h>
    #include <1541img/cbmdosvfs.h>
    #include <1541img/cbmdosvfsreader.h>
    #include <1541img/d64.h>
    #include <1541img/d64reader.h>
    #include <1541img/d64writer.h>
    #include <1541img/filedata.h>
    #include <1541img/hostfilereader.h>

    int main(void)
    {
        /* open file containing D64 */
        FILE *d64file = fopen("foo.d64", "rb");
        if (!d64file) return -1;

        /* read disk image from file */
        D64 *disk = readD64(d64file);
        fclose(d64file);
        if (!disk) return -1;

        /* determine matching filesystem options from disk image */
        CbmdosFsOptions opts = CFO_DEFAULT;
        if (probeCbmdosFsOptions(&opts, disk) < 0)
        {
            D64_destroy(disk);
            return -1;
        }

        /* create CbmdosFs instance and get VFS from it */
        CbmdosFs *fs = CbmdosFs_fromImage(disk, opts);
        if (!fs)
        {
            D64_destroy(disk);
            return -1;
        }
        CbmdosVfs *vfs = CbmdosFs_vfs(fs);

        /* read file to add from host file */
        FILE *hostfile = fopen("bar.prg", "rb");
        if (!hostfile)
        {
            CbmdosFs_destroy(fs);
            return -1;
        }
        FileData *filecontent = readHostFile(hostfile);
        fclose(hostfile);
        if (!filecontent)
        {
            CbmdosFs_destroy(fs);
            return -1;
        }

        /* create new CBM DOS file and set properties */
        CbmdosFile *file = CbmdosFile_create();
        CbmdosFile_setType(file, CFT_PRG);
        CbmdosFile_setName(file, "bar", 3);
        CbmdosFile_setData(file, filecontent);

        /* add new file to VFS */
        if (CbmdosVfs_append(vfs, file) < 0)
        {
            CbmdosFile_destroy(file);
            CbmdosFs_destroy(fs);
            return -1;
        }

        /* check consistent state of FS */
        if (CbmdosFs_status(fs) != CFS_OK)
        {
            CbmdosFs_destroy(fs);
            return -1;
        }

        /* save modified image */
        d64file = fopen("baz.d64", "wb");
        if (!d64file)
        {
            CbmdosFs_destroy(fs);
            return -1;
        }
        if (writeD64(d64file, CbmdosFs_image(fs)) < 0)
        {
            fclose(d64file);
            CbmdosFs_destroy(fs);
            return -1;
        }
        fclose(d64file);
        CbmdosFs_destroy(fs);
        return 0;
    }


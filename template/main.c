#include "../common/mmap.h"
#include "../common/print.h"
#include "../common/view.h"

int main(int argc, char** argv)
{
    mmap_file file = mmap_file_open_ro("input.txt");
    const int fileSize = (int)(file.size);

    //
    // content
    //

    return 0;
}

// Copy of simpledom example
// The allocation fixed memory buffers allocated on stack and prevents further heap access

#include <iostream>

#include "rapidjson/document.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

using namespace rapidjson;
using namespace std;

struct CustomAlloc {
    CustomAlloc(size_t  , unsigned char ) {
        RAPIDJSON_ASSERT(0);
    }
    CustomAlloc() {
    }
    void * Realloc(void * , size_t , size_t ) {
        RAPIDJSON_ASSERT(0);
    }
    static void Free(void *) {
        RAPIDJSON_ASSERT(0);
    }
    void * Malloc(size_t ) {
        RAPIDJSON_ASSERT(0);
    }
};

typedef MemoryPoolAllocator<CustomAlloc> myMemoryPoolAlloc;
typedef GenericDocument<UTF8<>, myMemoryPoolAlloc , myMemoryPoolAlloc > myDocument;
typedef GenericValue< UTF8<>, myMemoryPoolAlloc > myValue;

void insitu_parse(size_t chunk_size) {
    CustomAlloc crtAllocPreventer;
    unsigned char valueBuffer[120];
    unsigned char parseBuffer[160];
    myMemoryPoolAlloc valueAllocator(valueBuffer, sizeof(valueBuffer), sizeof(valueBuffer) , &crtAllocPreventer);
    myMemoryPoolAlloc parseAllocator(parseBuffer, sizeof(parseBuffer), sizeof(parseBuffer) , &crtAllocPreventer);

    // 1. Parse a JSON string into DOM.
    myDocument d(&valueAllocator, chunk_size, &parseAllocator);
    char json[] = "{\"project\":\"rapidjson\",\"stars\":10}";
    d.ParseInsitu(json);

    // 2. Modify it by DOM.
    myValue& s = d["stars"];

    // 3. Verify everything worked
    std::cout << "Stars:" << s.GetInt() << std::endl;
    std::cout << "Value alloc used:" << valueAllocator.Size() << std::endl;
    std::cout << "Parse alloc used:" << parseAllocator.Size() << std::endl;
    std::cout << "Chunk size:" << chunk_size << std::endl;
}

int main() {
    insitu_parse(124);
    return 0;
}

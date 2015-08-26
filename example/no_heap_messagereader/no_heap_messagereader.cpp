// Reading a message JSON with Reader (SAX-style API).
// The JSON should be an object with key-string pairs.

#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
#include <iostream>
#include <string>
#include <map>

using namespace std;
using namespace rapidjson;

typedef map<string, string> MessageMap;

#if defined(__GNUC__)
RAPIDJSON_DIAG_PUSH
RAPIDJSON_DIAG_OFF(effc++)
#endif

struct MessageHandler
    : public BaseReaderHandler<UTF8<>, MessageHandler> {
    MessageHandler() : messages_(), state_(kExpectObjectStart), name_() {}

    bool StartObject() {
        switch (state_) {
        case kExpectObjectStart:
            state_ = kExpectNameOrObjectEnd;
            return true;
        default:
            return false;
        }
    }

    bool String(const char* str, SizeType length, bool) {
        switch (state_) {
        case kExpectNameOrObjectEnd:
            name_ = string(str, length);
            state_ = kExpectValue;
            return true;
        case kExpectValue:
            messages_.insert(MessageMap::value_type(name_, string(str, length)));
            state_ = kExpectNameOrObjectEnd;
            return true;
        default:
            return false;
        }
    }

    bool EndObject(SizeType) { return state_ == kExpectNameOrObjectEnd; }

    bool Default() { return false; } // All other events are invalid.

    MessageMap messages_;
    enum State {
        kExpectObjectStart,
        kExpectNameOrObjectEnd,
        kExpectValue
    }state_;
    std::string name_;
};

#if defined(__GNUC__)
RAPIDJSON_DIAG_POP
#endif

struct PreventCrtAlloc {
    PreventCrtAlloc(size_t  , unsigned char ) {
        RAPIDJSON_ASSERT(0);
    }
    PreventCrtAlloc() {
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

typedef MemoryPoolAllocator<PreventCrtAlloc> myPoolAlloc;
typedef GenericReader<UTF8<>, UTF8<>, myPoolAlloc> myReader;

void ParseMessages(const char* json, MessageMap& messages) {
    PreventCrtAlloc crtAllocPreventer;
    unsigned char parseBuffer[48];
    size_t chunk_1 = 20;
    size_t chunk_2 = 20;
    myPoolAlloc parseAllocator(parseBuffer, sizeof(parseBuffer),
                               chunk_1 , &crtAllocPreventer);
    
    myReader reader(&parseAllocator, chunk_2);

    MessageHandler handler;
    StringStream ss(json);
    if (reader.Parse(ss, handler))
        messages.swap(handler.messages_);   // Only change it if success.
    else {
        ParseErrorCode e = reader.GetParseErrorCode();
        size_t o = reader.GetErrorOffset();
        cout << "Error: " << GetParseError_En(e) << endl;;
        cout << " at offset " << o << " near '" << string(json).substr(o, 10) << "...'" << endl;
    }
    std::cout << "Used alloc size:" << parseAllocator.Size() << std::endl;
}

int main() {
    MessageMap messages;

    const char* json1 = "{ \"greeting\" : \"Hello!\", \"farewell\" : \"bye-bye!\" }";
    cout << json1 << endl;
    ParseMessages(json1, messages);

    for (MessageMap::const_iterator itr = messages.begin(); itr != messages.end(); ++itr)
        cout << itr->first << ": " << itr->second << endl;

    cout << endl << "Parse a JSON with invalid schema." << endl;
    const char* json2 = "{ \"greeting\" : \"Hello!\", \"farewell\" : \"bye-bye!\", \"foo\" : {} }";
    cout << json2 << endl;
    ParseMessages(json2, messages);

    return 0;
}

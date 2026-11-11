#ifndef PORT_INPUT_SOURCES_H
#define PORT_INPUT_SOURCES_H
#include <stdint.h>
// Shared key/button ownership only; contains no pointer gesture state.
static inline uint32_t PortInputSources(uint32_t previous,unsigned source,int held) {
    if(source>=32)return previous;
    uint32_t bit=1u<<source;return held?(previous|bit):(previous&~bit);
}
#endif

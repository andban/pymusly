#ifndef MUSLY_DECODERS_COREAUDIO_H_
#define MUSLY_DECODERS_COREAUDIO_H_

#include "decoder.h"

namespace musly { namespace decoders {

class coreaudio : public musly::decoder
{
    MUSLY_DECODER_REGCLASS(coreaudio);

public:
    coreaudio();

    virtual std::vector<float>
    decodeto_22050hz_mono_float(const std::string& file, float excerpt_length, float excerpt_start);
};

}} // namespace musly::decoders

#endif /* MUSLY_DECODERS_COREAUDIO_H_  */

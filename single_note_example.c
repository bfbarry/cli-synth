#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>

#define SAMPLE_RATE 44100
#define FREQ 261.63
#define TWO_PI (2.0 * M_PI)
typedef struct {
    double phase;
} SineWave;

OSStatus renderCallback(
    void *inRefCon,
    AudioUnitRenderActionFlags *ioActionFlags,
    const AudioTimeStamp *inTimeStamp,
    UInt32 inBusNumber,
    UInt32 inNumberFrames,
    AudioBufferList *ioData
) {
    SineWave *sineWave = (SineWave *)inRefCon;
    Float32 *buffer = (Float32 *)ioData->mBuffers[0].mData;

    for (UInt32 frame = 0; frame < inNumberFrames; frame++) {
        buffer[frame] = (Float32)(0.5 * sin(sineWave->phase));
        sineWave->phase += TWO_PI * FREQ / SAMPLE_RATE;
        if (sineWave->phase >= TWO_PI)
            sineWave->phase -= TWO_PI;
    }

    return noErr;
}

int main() {
    AudioComponentInstance audioUnit;
    AudioComponentDescription desc = {0};
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;

    AudioComponent outputComponent = AudioComponentFindNext(NULL, &desc);
    AudioComponentInstanceNew(outputComponent, &audioUnit);

    AURenderCallbackStruct callbackStruct;
    SineWave sineWave = {0};

    callbackStruct.inputProc = renderCallback;
    callbackStruct.inputProcRefCon = &sineWave;

    AudioUnitSetProperty(audioUnit,
                         kAudioUnitProperty_SetRenderCallback,
                         kAudioUnitScope_Input,
                         0,
                         &callbackStruct,
                         sizeof(callbackStruct));

    AudioStreamBasicDescription streamDesc = {0};
    streamDesc.mSampleRate = SAMPLE_RATE;
    streamDesc.mFormatID = kAudioFormatLinearPCM;
    streamDesc.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    streamDesc.mFramesPerPacket = 1;
    streamDesc.mChannelsPerFrame = 1;
    streamDesc.mBitsPerChannel = 32;
    streamDesc.mBytesPerFrame = 4;
    streamDesc.mBytesPerPacket = 4;

    AudioUnitSetProperty(audioUnit,
                         kAudioUnitProperty_StreamFormat,
                         kAudioUnitScope_Input,
                         0,
                         &streamDesc,
                         sizeof(streamDesc));

    AudioUnitInitialize(audioUnit);
    AudioOutputUnitStart(audioUnit);

    printf("Playing middle C (261.63 Hz)... press Enter to stop.\n");
    getchar();

    AudioOutputUnitStop(audioUnit);
    AudioUnitUninitialize(audioUnit);
    AudioComponentInstanceDispose(audioUnit);

    return 0;
}

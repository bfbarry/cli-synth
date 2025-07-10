// audio stuff
#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>
// keyboard stuff
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>


pthread_mutex_t lock;

#define SAMPLE_RATE 44100
static const char COMPUTER_KEYS[] = "-qwertyuiopasd-";
char KEYS_ACTIVE_DISPLAY[]        = "|-------------|";
// static const char* NOTES_STRS = { "C4", "Cs4", "D4", "Ds4", "E4", "F4", "Fs4", "G4", "Gs4", "A4", "As4", "B4", "C5"};
// char NOTES_ACTIVE_DISPLAY[]       = "|-------------|";

enum Notes {
    LowBound,
    C4,
    Cs4,
    D4,
    Ds4,
    E4,
    F4,
    Fs4,
    G4,
    Gs4,
    A4,
    As4,
    B4,
    C5,
    UpBound
};

Float32 FREQTABLE[UpBound-1];

void initFreqTable(Float32* freqs) {
    // TODO: think of a better, abstract way to do this
    Float32 A4v = 440.0f; // A4 is 440 Hz
    Float32 C4v = A4v * powf(2.0f, -9.0f / 12.0f); // C4 is 9 semitones below A4
    Float32 Cs4v = C4v * powf(2.0f, 1.0f / 12.0f); // C#4 is 1 semitone above C4
    Float32 D4v = C4v * powf(2.0f, 2.0f / 12.0f); // D4 is 2 semitones above C4
    Float32 Ds4v = C4v * powf(2.0f, 3.0f / 12.0f); // D#4 is 3 semitones above C4
    Float32 E4v = C4v * powf(2.0f, 4.0f / 12.0f); // E4 is 4 semitones above C4
    Float32 F4v = C4v * powf(2.0f, 5.0f / 12.0f); // F4 is 5 semitones above C4
    Float32 Fs4v = C4v * powf(2.0f, 6.0f / 12.0f); // F#4 is 6 semitones above C4
    Float32 G4v = C4v * powf(2.0f, 7.0f / 12.0f); // G4 is 7 semitones above C4
    Float32 Gs4v = C4v * powf(2.0f, 8.0f / 12.0f); // G#4 is 8 semitones above C4
    Float32 As4v = A4v * powf(2.0f, 1.0f / 12.0f); // A#4 is 1 semitone above A4
    Float32 B4v = A4v * powf(2.0f, 2.0f / 12.0f); // B4 is 2 semitones above A4
    Float32 C5v = A4v * powf(2.0f, 3.0f / 12.0f); // C5 is 3 semitones above A4

    freqs[C4] = C4v;
    freqs[Cs4] = Cs4v;
    freqs[D4] = D4v;
    freqs[Ds4] = Ds4v;
    freqs[E4] = E4v;
    freqs[F4] = F4v;
    freqs[Fs4] = Fs4v;
    freqs[G4] = G4v;
    freqs[Gs4] = Gs4v;
    freqs[A4] = A4v;
    freqs[As4] = As4v;
    freqs[B4] = B4v;
    freqs[C5] = C5v;
}

typedef struct {
    bool keyStatus[UpBound-1];
    double phases[UpBound-1];
} SynthState;

// mut
Float32 stepForward(SynthState* this) {
    Float32 signal_t = 0.;
    for (int noteIx=LowBound+1; noteIx < UpBound; ++noteIx) {
        if (this->keyStatus[noteIx]) {
            signal_t += (Float32)(0.5 * sin(this->phases[noteIx]));
            this->phases[noteIx] += (2.0 * M_PI) * (FREQTABLE[noteIx]) / SAMPLE_RATE ;
            if (this->phases[noteIx] >= (2.0 * M_PI)) {
                this->phases[noteIx]  -= (2.0 * M_PI);
            }
        }
    }
    return signal_t; 
}

OSStatus renderCallback(
    void* inRefCon,
    AudioUnitRenderActionFlags* ioActionFlags,
    const AudioTimeStamp* inTimeStamp,
    UInt32 inBusNumber,
    UInt32 inNumberFrames,
    AudioBufferList* ioData
) {
    SynthState* synthState = (SynthState*)inRefCon;
    Float32* buffer = (Float32*)ioData->mBuffers[0].mData;
    pthread_mutex_lock(&lock);
    for (UInt32 frame=0; frame < inNumberFrames; frame++) {
        Float32 signal_t = stepForward(synthState);
        // printf("FREQ %d ", freq);
        buffer[frame] = signal_t;
    }

    pthread_mutex_unlock(&lock);
    return noErr;
}

// Keyboard stuff 
void setNonCanonicalMode(int enable) {
    static struct termios oldt, newt;
    if (enable) {
        tcgetattr(STDIN_FILENO, &oldt);
        newt = oldt;
        newt.c_lflag &= ~(ICANON | ECHO);
        tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    } else {
        tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    }
}

int kbhit(void) {
    struct termios oldt, newt;
    int ch;
    int oldf;
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}

void* detect_keys(void* arg) {
    setNonCanonicalMode(1);

    printf("Press '0' to quit.\n");
    SynthState* state = (SynthState*) arg;

    while (1) {
        printf("\r%s", KEYS_ACTIVE_DISPLAY);
        fflush(stdout);
        if (kbhit()) {
            char c = getchar();
            if (c == '0')
                break;
            
            pthread_mutex_lock(&lock);

            for (int noteIx=LowBound+1; noteIx < UpBound; ++noteIx) {
                if (c == COMPUTER_KEYS[noteIx]) {
                    state->keyStatus[noteIx] = state->keyStatus[noteIx] ? false : true;
                    if (state->keyStatus[noteIx]) {
                        KEYS_ACTIVE_DISPLAY[noteIx] = c;
                    } else {
                        KEYS_ACTIVE_DISPLAY[noteIx] = '-';
                    }
                }
            }
            pthread_mutex_unlock(&lock);   // release lock
        }
        usleep(10000);
    }

    setNonCanonicalMode(0);
    return NULL;
}


int main() {
    initFreqTable(FREQTABLE);

    // set callback
    AudioComponentDescription desc = {0};
    desc.componentType = kAudioUnitType_Output;
    desc.componentSubType = kAudioUnitSubType_DefaultOutput;
    desc.componentManufacturer = kAudioUnitManufacturer_Apple;

    AudioComponent outputComponent = AudioComponentFindNext(NULL, &desc);
    AudioComponentInstance audioUnit;
    AudioComponentInstanceNew(outputComponent, &audioUnit);

    AURenderCallbackStruct callbackStruct;
    SynthState synthState = {0};
    // synthState.
    callbackStruct.inputProc = renderCallback;
    callbackStruct.inputProcRefCon = &synthState;

    AudioUnitSetProperty(
        audioUnit,
        kAudioUnitProperty_SetRenderCallback,
        kAudioUnitScope_Input,
        0,
        &callbackStruct,
        sizeof(callbackStruct)
            );

    // set metadata
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
                        sizeof(streamDesc)
                );

    // keyboard stuff
    pthread_t user_input_thread;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex init failed\n");
        return 1;
    }
    pthread_create(&user_input_thread, NULL, detect_keys, &synthState);

    AudioUnitInitialize(audioUnit);
    AudioOutputUnitStart(audioUnit);

    pthread_join(user_input_thread, NULL);
    pthread_mutex_destroy(&lock);

    AudioOutputUnitStop(audioUnit);
    AudioUnitUninitialize(audioUnit);
    AudioComponentInstanceDispose(audioUnit);

    return 0;
}
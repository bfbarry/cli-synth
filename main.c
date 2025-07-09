#include <AudioToolbox/AudioToolbox.h>
#include <math.h>
#include <stdio.h>

// key press stuff
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include <pthread.h>


pthread_mutex_t lock;

#define SAMPLE_RATE 44100
#define FREQ 261.3

typedef struct {
    bool C4;
    bool Cs4;
    bool D4;
    bool Ds4;
    bool E4;
    bool F4;
    bool Fs4;
    bool G4;
    bool Gs4;
    bool A4;
    bool As4;
    bool B4;
    bool C5;
} KeyStatus;

typedef struct {
    Float32 C4;
    Float32 Cs4;
    Float32 D4;
    Float32 Ds4;
    Float32 E4;
    Float32 F4;
    Float32 Fs4;
    Float32 G4;
    Float32 Gs4;
    Float32 A4;
    Float32 As4;
    Float32 B4;
    Float32 C5;
} FreqTable;

// TODO use array and enum 
typedef struct {
    double C4;
    double Cs4;
    double D4;
    double Ds4;
    double E4;
    double F4;
    double Fs4;
    double G4;
    double Gs4;
    double A4;
    double As4;
    double B4;
    double C5;
} PhaseTable;

void initFreqTable(FreqTable* self) {
    // TODO: think of a better, abstract way to do this
    Float32 A4 = 440.0f; // A4 is 440 Hz
    Float32 C4 = A4 * powf(2.0f, -9.0f / 12.0f); // C4 is 9 semitones below A4
    Float32 Cs4 = C4 * powf(2.0f, 1.0f / 12.0f); // C#4 is 1 semitone above C4
    Float32 D4 = C4 * powf(2.0f, 2.0f / 12.0f); // D4 is 2 semitones above C4
    Float32 Ds4 = C4 * powf(2.0f, 3.0f / 12.0f); // D#4 is 3 semitones above C4
    Float32 E4 = C4 * powf(2.0f, 4.0f / 12.0f); // E4 is 4 semitones above C4
    Float32 F4 = C4 * powf(2.0f, 5.0f / 12.0f); // F4 is 5 semitones above C4
    Float32 Fs4 = C4 * powf(2.0f, 6.0f / 12.0f); // F#4 is 6 semitones above C4
    Float32 G4 = C4 * powf(2.0f, 7.0f / 12.0f); // G4 is 7 semitones above C4
    Float32 Gs4 = C4 * powf(2.0f, 8.0f / 12.0f); // G#4 is 8 semitones above C4
    Float32 As4 = A4 * powf(2.0f, 1.0f / 12.0f); // A#4 is 1 semitone above A4
    Float32 B4 = A4 * powf(2.0f, 2.0f / 12.0f); // B4 is 2 semitones above A4
    Float32 C5 = A4 * powf(2.0f, 3.0f / 12.0f); // C5 is 3 semitones above A4

    self->C4 = C4;
    self->Cs4 = Cs4;
    self->D4 = D4;
    self->Ds4 = Ds4;
    self->E4 = E4;
    self->F4 = F4;
    self->Fs4 = Fs4;
    self->G4 = G4;
    self->Gs4 = Gs4;
    self->A4 = A4;
    self->As4 = As4;
    self->B4 = B4;
    self->C5 = C5;
}
FreqTable FREQTABLE;

typedef struct {
    KeyStatus keyStatus;
    PhaseTable phases;
} SynthState;


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
        Float32 signal_t = 0.;

        if (synthState->keyStatus.C4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.C4));
            synthState->phases.C4 += (2.0 * M_PI) * (FREQTABLE.C4) / SAMPLE_RATE ;
            if (synthState->phases.C4 >= (2.0 * M_PI)) {
                synthState->phases.C4  -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.Cs4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.Cs4));
            synthState->phases.Cs4 += (2.0 * M_PI) * (FREQTABLE.Cs4) / SAMPLE_RATE ;
            if (synthState->phases.Cs4 >= (2.0 * M_PI)) {
                synthState->phases.Cs4 -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.D4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.D4));
            synthState->phases.D4 += (2.0 * M_PI) * (FREQTABLE.D4) / SAMPLE_RATE ;
            if (synthState->phases.D4 >= (2.0 * M_PI)) {
                synthState->phases.D4  -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.Ds4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.Ds4));
            synthState->phases.Ds4 += (2.0 * M_PI) * (FREQTABLE.Ds4) / SAMPLE_RATE ;
            if (synthState->phases.Ds4 >= (2.0 * M_PI)) {
                synthState->phases.Ds4 -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.E4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.E4));
            synthState->phases.E4 += (2.0 * M_PI) * (FREQTABLE.E4) / SAMPLE_RATE ;
            if (synthState->phases.E4 >= (2.0 * M_PI)) {
                synthState->phases.E4  -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.F4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.F4));
            synthState->phases.F4 += (2.0 * M_PI) * (FREQTABLE.F4) / SAMPLE_RATE ;
            if (synthState->phases.F4 >= (2.0 * M_PI)) {
                synthState->phases.F4  -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.Fs4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.Fs4));
            synthState->phases.Fs4 += (2.0 * M_PI) * (FREQTABLE.Fs4) / SAMPLE_RATE ;
            if (synthState->phases.Fs4 >= (2.0 * M_PI)) {
                synthState->phases.Fs4 -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.G4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.G4));
            synthState->phases.G4 += (2.0 * M_PI) * (FREQTABLE.G4) / SAMPLE_RATE ;
            if (synthState->phases.G4 >= (2.0 * M_PI)) {
                synthState->phases.G4  -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.Gs4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.Gs4));
            synthState->phases.Gs4 += (2.0 * M_PI) * (FREQTABLE.Gs4) / SAMPLE_RATE ;
            if (synthState->phases.Gs4 >= (2.0 * M_PI)) {
                synthState->phases.Gs4 -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.A4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.A4));
            synthState->phases.A4 += (2.0 * M_PI) * (FREQTABLE.A4) / SAMPLE_RATE ;
            if (synthState->phases.A4 >= (2.0 * M_PI)) {
                synthState->phases.A4  -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.As4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.As4));
            synthState->phases.As4 += (2.0 * M_PI) * (FREQTABLE.As4) / SAMPLE_RATE ;
            if (synthState->phases.As4 >= (2.0 * M_PI)) {
                synthState->phases.As4 -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.B4) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.B4));
            synthState->phases.B4 += (2.0 * M_PI) * (FREQTABLE.B4) / SAMPLE_RATE ;
            if (synthState->phases.B4 >= (2.0 * M_PI)) {
                synthState->phases.B4  -= (2.0 * M_PI);
            }
        } if (synthState->keyStatus.C5) {
            signal_t += (Float32)(0.5 * sin(synthState->phases.C5));
            synthState->phases.C5 += (2.0 * M_PI) * (FREQTABLE.C5) / SAMPLE_RATE ;
            if (synthState->phases.C5 >= (2.0 * M_PI)) {
                synthState->phases.C5  -= (2.0 * M_PI);
            }
        }
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
    // TODO: repeat keys to turn them off
    setNonCanonicalMode(1);

    printf("Press any key to test. Press '0' to quit.\n");
    SynthState* state = (SynthState*) arg;

    while (1) {
        if (kbhit()) {
            char c = getchar();
            printf("Key: %c\n", c);
            printf("Hello!");  
            if (c == '0')
                break;
            pthread_mutex_lock(&lock);     // acquire lock

            switch (c) {
                case 'q':
                    state->keyStatus.C4 = true;
                    break;
                case 'w':
                    state->keyStatus.Cs4 = true;
                    break;
                case 'e':
                    state->keyStatus.D4 = true;
                    break;
                case 'r':
                    state->keyStatus.Ds4 = true;
                    break;
                case 't':
                    state->keyStatus.E4 = true;
                    break;
                case 'y':
                    state->keyStatus.F4 = true;
                    break;
                case 'u':
                    state->keyStatus.Fs4 = true;
                    break;
                case 'i':
                    state->keyStatus.G4 = true;
                    break;
                case 'o':
                    state->keyStatus.Gs4 = true;
                    break;
                case 'p':
                    state->keyStatus.A4 = true;
                    break;
                case 'a':
                    state->keyStatus.As4 = true;
                    break;
                case 's':
                    state->keyStatus.B4 = true;
                    break;
                case 'd':
                    state->keyStatus.C5 = true;
                    break;
            }
            pthread_mutex_unlock(&lock);   // release lock
        }
        usleep(10000);
    }

    setNonCanonicalMode(0);
    return NULL;
}


int main() {
    initFreqTable(&FREQTABLE);

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



    // keyboards stuff
    pthread_t user_input_thread;
    if (pthread_mutex_init(&lock, NULL) != 0) {
        printf("Mutex init failed\n");
        return 1;
    }


    pthread_create(&user_input_thread, NULL, detect_keys, &synthState);

    AudioUnitInitialize(audioUnit);
    AudioOutputUnitStart(audioUnit);
    printf("playing note enter to stop");
    // getchar();

    pthread_join(user_input_thread, NULL);
    pthread_mutex_destroy(&lock);

    AudioOutputUnitStop(audioUnit);
    AudioUnitUninitialize(audioUnit);
    AudioComponentInstanceDispose(audioUnit);

    return 0;
}
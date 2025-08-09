#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

// Morse code definitions
typedef struct {
    const char *morse;
    char character;
} MorseCode;

MorseCode morse_table[] = {
    {".-", 'A'}, {"-...", 'B'}, {"-.-.", 'C'}, {"-..", 'D'}, {".", 'E'},
    {"..-.", 'F'}, {"--.", 'G'}, {"....", 'H'}, {"..", 'I'}, {".---", 'J'},
    {"-.-", 'K'}, {".-..", 'L'}, {"--", 'M'}, {"-.", 'N'}, {"---", 'O'},
    {".--.", 'P'}, {"--.-", 'Q'}, {".-.", 'R'}, {"...", 'S'}, {"-", 'T'},
    {"..-", 'U'}, {"...-", 'V'}, {".--", 'W'}, {"-..-", 'X'}, {"-.--", 'Y'},
    {"--..", 'Z'}, {"-----", '0'}, {".----", '1'}, {"..---", '2'},
    {"...--", '3'}, {"....-", '4'}, {".....", '5'}, {"-....", '6'},
    {"--...", '7'}, {"---..", '8'}, {"----.", '9'}, {"-.-.--", '!'},
    {".-.-.-", '.'}, {"--..--", ','}, {"..-..", '?'}, {"-..-.", '/'},
    {"-....-", '-'}, {".--.-.", '@'}, {"---...", ':'}, {"-.-.-.", ';'},
    {"-.-.  -.", 'K'}, {"--.", 'G'}
};

#define FREQUENCY 700 // Tone frequency in Hz
#define SAMPLE_RATE 44100
#define DIT_DURATION_MS 100 // 100ms per dit
#define SAMPLES_PER_DIT (SAMPLE_RATE * DIT_DURATION_MS / 1000)
#define BUFFER_SIZE SAMPLES_PER_DIT

// Audio buffer for the sine wave
Sint16 audio_buffer[BUFFER_SIZE];

// Global audio device handle
SDL_AudioDeviceID audio_device;

// Global flag to stop playback
int stop_playback = 0;

void generate_sine_wave(Sint16 *buffer, int length) {
    for (int i = 0; i < length; i++) {
        buffer[i] = (Sint16)(32767.0 * sin(2.0 * M_PI * FREQUENCY * i / SAMPLE_RATE));
    }
}

void play_morse_tone(int duration_ms) {
    int samples_to_play = (int)((double)duration_ms / 1000.0 * SAMPLE_RATE);
    while (samples_to_play > 0 && !stop_playback) {
        int count = samples_to_play > BUFFER_SIZE ? BUFFER_SIZE : samples_to_play;
        if (SDL_QueueAudio(audio_device, audio_buffer, count * sizeof(Sint16)) != 0) {
            printf("Failed to queue audio: %s\n", SDL_GetError());
            break;
        }
        SDL_Delay((int)((double)count / SAMPLE_RATE * 1000.0));
        samples_to_play -= count;
    }
}

void send_morse(const char *message) {
    printf("Sending: %s\n", message);
    for (int i = 0; message[i] != '\0' && !stop_playback; i++) {
        char ch = toupper(message[i]);
        if (ch == ' ') {
            // Space between words (7 dits)
            printf(" ");
            SDL_Delay(DIT_DURATION_MS * 7);
            continue;
        }

        // Find the Morse code for the character
        const char *morse_code = NULL;
        for (int j = 0; j < sizeof(morse_table) / sizeof(morse_table[0]); j++) {
            if (morse_table[j].character == ch) {
                morse_code = morse_table[j].morse;
                break;
            }
        }
        if (morse_code == NULL) {
            continue; // Skip unknown characters
        }

        // Send the dots and dashes
        for (int j = 0; morse_code[j] != '\0' && !stop_playback; j++) {
            if (morse_code[j] == '.') {
                printf(".");
                play_morse_tone(DIT_DURATION_MS);
                SDL_Delay(DIT_DURATION_MS); // Gap between dots/dashes
            } else if (morse_code[j] == '-') {
                printf("-");
                play_morse_tone(DIT_DURATION_MS * 3);
                SDL_Delay(DIT_DURATION_MS); // Gap between dots/dashes
            }
        }
        printf(" ");
        SDL_Delay(DIT_DURATION_MS * 3); // Gap between characters (3 dits)
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    // Initialize both audio and video subsystems to handle audio and keyboard events
    if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO) != 0) {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return 1;
    }

    // Creating a window is not strictly necessary for this app, but
    // it ensures the event loop works correctly on all platforms.
    SDL_Window* window = SDL_CreateWindow("Morse Sender", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 100, 100, SDL_WINDOW_HIDDEN);
    if (window == NULL) {
        printf("SDL_CreateWindow Error: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    
    printf("Sending Morse code... Press Ctrl+C in the terminal to exit.\n");
    
    SDL_AudioSpec want, have;
    SDL_zero(want);
    want.freq = SAMPLE_RATE;
    want.format = AUDIO_S16;
    want.channels = 1;
    want.samples = BUFFER_SIZE;
    want.callback = NULL;

    audio_device = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0);
    if (audio_device == 0) {
        printf("Failed to open audio device: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 1;
    }

    generate_sine_wave(audio_buffer, BUFFER_SIZE);
    SDL_PauseAudioDevice(audio_device, 0); // Unpause audio

    const char *message = "CQ CQ CQ DX 2E0ODJ";
    Uint32 last_message_time = SDL_GetTicks();
    const Uint32 message_interval = 5000; // 5 second interval

    while (!stop_playback) {
        // Handle events first to ensure responsiveness
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                stop_playback = 1;
            }
            if (event.type == SDL_KEYDOWN) {
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    stop_playback = 1;
                }
            }
        }
        
        // Check if it's time to send the next message
        if (SDL_GetTicks() - last_message_time >= message_interval) {
            send_morse(message);
            last_message_time = SDL_GetTicks();
        }

        SDL_Delay(10); // A small delay to avoid 100% CPU usage
    }
    
    SDL_CloseAudioDevice(audio_device);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

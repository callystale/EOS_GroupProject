// Add this helper function to check if timer has expired without waiting
int is_timer_expired() {
    register unsigned long r;
    static unsigned long expiredTime = 0;
    
    // Get the static expiredTime from set_wait_timer by accessing it
    // We'll use a global variable instead for cleaner implementation
    asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
    extern unsigned long global_timer_expire; // declare this global
    return (r >= global_timer_expire);
}

// Add this global variable at the top of your file (outside functions)
unsigned long global_timer_expire = 0;

// Modified set_wait_timer function to work with our timer checking
void set_shooting_timer(unsigned int msVal) {
    register unsigned long f, t;
    // Get the current counter frequency (Hz)
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    // Read the current counter
    asm volatile ("mrs %0, cntpct_el0" : "=r"(t));
    // Calculate expired time and store globally
    global_timer_expire = t + f * msVal / 1000;
}

int check_shooting_timer_expired() {
    register unsigned long r;
    asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
    return (r >= global_timer_expire);
}

// Get remaining time in milliseconds
unsigned int get_remaining_time_ms() {
    register unsigned long r, f;
    asm volatile ("mrs %0, cntpct_el0" : "=r"(r));
    asm volatile ("mrs %0, cntfrq_el0" : "=r"(f));
    
    if (r >= global_timer_expire) {
        return 0; // Timer expired
    }
    
    unsigned long remaining_ticks = global_timer_expire - r;
    return (unsigned int)(remaining_ticks * 1000 / f);
}

// Simple integer to string conversion
void intToStr(unsigned int num, char *str) {
    char buf[12];
    int i = 0;
    if (num == 0) {
        str[0] = '0';
        str[1] = '\0';
        return;
    }
    while (num > 0) {
        buf[i++] = '0' + (num % 10);
        num /= 10;
    }
    // reverse into str
    for (int j = 0; j < i; j++) {
        str[j] = buf[i - j - 1];
    }
    str[i] = '\0';
}

// String concatenation helper
char* strcopy(char *dest, const char *src) {
    while (*src) *dest++ = *src++;
    *dest = '\0';
    return dest;
}

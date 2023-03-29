struct s_prime
{
    int number;
    double time;
};

typedef struct s_prime* Prime;

void write_safe(int fd,  void* buf, int len);

void read_safe(int fd,  void* buf, int len);
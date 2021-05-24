#ifndef __LIBSPEEDWIRE_SPEEDWIRESOCKETSIMPLE_HPP__
#define __LIBSPEEDWIRE_SPEEDWIRESOCKETSIMPLE_HPP__

namespace libspeedwire {

    class SpeedwireSocketSimple {

    protected:
        static const char* multicast_group;
        static const int   multicast_port;
        static int fd;
        static SpeedwireSocketSimple* instance;

    private:
        SpeedwireSocketSimple(void);
        ~SpeedwireSocketSimple(void);
        int open(void);     // call getInstance() instead

    public:
        static SpeedwireSocketSimple* getInstance(void);
        int send(const void* const buff, const unsigned long size);
        int sendto(const void* const buff, const unsigned long size, const struct sockaddr_in& dest);
        int recv(void* buff, const unsigned long size);
        int recvfrom(void* buff, const unsigned long size, struct sockaddr_in& src);
        int close(void);
    };

}   // namespace libspeedwire

#endif

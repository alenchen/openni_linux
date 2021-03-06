#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define IMG_WIDTH 640
#define IMG_HEIGHT 480
#define SERVER_PORT 5566
#define CLIENT_PORT 5567
#define MAX_DATA 1024
#define PKT_SIZE 51200
#define MAX_COUNT 20

using namespace std;
using namespace cv;

int main( int argc, char* argv[] )
{
    int socket_fd;
    int size;
    int frame_size = IMG_WIDTH*IMG_HEIGHT*2;
    char data[MAX_DATA] = {0};
    char *data1;
    data1 = (char*)malloc((PKT_SIZE+1)*sizeof(char));
    char *data_all;
    data_all = (char*)malloc(IMG_WIDTH*IMG_HEIGHT*2*sizeof(char));
    struct sockaddr_in myaddr;
    struct sockaddr_in server_addr;
    struct hostent *hp;

    if ( argc < 3 ) {
        cout << "No parameter" << endl;
        return 1;
    }

    if ((socket_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0 ) {
        cout << "socket failed" << endl;
        return 1;
    }

    memset(&myaddr, 0, sizeof(myaddr));
    memset(&server_addr, 0, sizeof(server_addr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(CLIENT_PORT);
    if (bind(socket_fd, (struct sockaddr*)&myaddr, sizeof(myaddr)) < 0) {
        cout << "bind socket failed" << endl;
        return 1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    hp = gethostbyname(argv[1]);
    if (hp == 0) {
        cout << "could not obtain address" << endl;
        return 1;
    }
    memcpy(hp->h_addr_list[0], (caddr_t)&server_addr.sin_addr, hp->h_length); 
    strncpy(data, argv[2], MAX_DATA); 

    size = sizeof(server_addr);
    if (sendto(socket_fd, data, sizeof(data), 0, (struct sockaddr*)&server_addr, size) == -1) {
        cout << "sendto server error" << endl;
        return 1;
    } 

    unsigned count = 0;
    while ( true )
    {
        recvfrom(socket_fd, data1, PKT_SIZE + 1, 0, (struct sockaddr*)&server_addr, (socklen_t *)&size);
        unsigned int offset = *data1 - 'A';
        //cout << "offset : " << offset << endl;
        memcpy(data_all + offset * PKT_SIZE , data1 + 1, PKT_SIZE);
        if (++count < MAX_COUNT) {
            continue;
        } else {
            count = 0;
        }
        Mat imgDepth( IMG_HEIGHT, IMG_WIDTH, CV_16UC1, ( void* )data_all );
        Mat img8bitDepth;
        imgDepth.convertTo( img8bitDepth, CV_8U, 255.0 / 5000 );
        imshow( "Depth view", img8bitDepth );

        waitKey( 1 );
    }

    free(data1);
    free(data_all);
    return 0;
}

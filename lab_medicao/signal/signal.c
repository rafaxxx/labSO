
/* Example of use of a Linux-specific call - signalfd() to handle signals using
 * a file descriptor. - http://www.linuxprogrammingblog.com/code-examples/signalfd
 */

#include <unistd.h>
#include <signal.h>
#include <sys/signalfd.h>
#include <string.h>

int main (int argc, char *argv[])
{
        int sfd;
        sigset_t mask;

        /* We will handle SIGTERM and SIGINT. */
        sigemptyset (&mask);
        sigaddset (&mask, SIGINT);
        sigaddset (&mask, SIGTERM);
        sigaddset (&mask, SIGQUIT);


         printf("My process ID : %d\n", getpid());
        /* Block the signals thet we handle using signalfd(), so they don't
         * cause signal handlers or default signal actions to execute. */
        sigprocmask(SIG_BLOCK, &mask, NULL);

        /* Create a file descriptor from which we will read the signals. */
        sfd = signalfd (-1, &mask, 0);

        while (1) {

                /** The buffor for read(), this structure contains information
                 * about the signal we've read. */
                struct signalfd_siginfo si;

                ssize_t res;

                res = read (sfd, &si, sizeof(si));

                if (si.ssi_signo == SIGTERM){
                        printf ("Got SIGTERM\n");
                }
                else if (si.ssi_signo == SIGINT) {
                        printf ("Bye!\n");
                        break;
                }
                else if (si.ssi_signo == SIGQUIT) {
                        printf ("Nem Morreu!\n");
                }
        }
        /* Close the file descriptor if we no longer need it. */
        close (sfd);
        return 0;
}

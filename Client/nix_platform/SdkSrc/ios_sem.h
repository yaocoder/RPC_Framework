#ifndef sem_ios_h__
#define sem_ios_h__

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <semaphore.h>

sem_t * CreateSemaphore(const char * inName, const int inStartingCount);
bool DestroySemaphore(sem_t * inSemaphore);
void SignalSemaphore(sem_t * inSemaphore);
int WaitSemaphore( sem_t * inSemaphore );
bool TryWaitSemaphore( sem_t * inSemaphore );
bool ClearSemaphore(const char * inName);

#endif

sem_t * CreateSemaphore(const char * inName, const int inStartingCount) {
	sem_t * semaphore = sem_open(inName, O_CREAT, 0644, inStartingCount);

	if (semaphore == SEM_FAILED ) {
		switch (errno) {
		case EEXIST:
			printf("Semaphore with name '%s' already exists.\n", inName);
			break;

		default:
			printf("Unhandled error: %d.\n", errno);
			break;
		}

		assert(false);
		return SEM_FAILED ;
	}

	return semaphore;
}

bool DestroySemaphore(sem_t * inSemaphore) {
	int retErr = sem_close(inSemaphore);

	if (retErr == -1) {
		switch (errno) {
		case EINVAL:
			printf("inSemaphore is not a valid sem_t object.");
			break;

		default:
			printf("Unhandled error: %d.\n", errno);
			break;
		}

		assert(false);
		return false;
	}

	return true;
}

void SignalSemaphore(sem_t * inSemaphore) {
	sem_post(inSemaphore);
}

int WaitSemaphore( sem_t * inSemaphore )
{
    return sem_wait( inSemaphore );
}

bool TryWaitSemaphore( sem_t * inSemaphore )
{
    int retErr = sem_trywait( inSemaphore );

    if( retErr == -1 )
    {
        if( errno != EAGAIN )
        {
            printf( "Unhandled error: %d\n", errno );
            assert( false );
        }

        return false;
    }

    return true;
}

bool ClearSemaphore(const char * inName) {
	int retErr = sem_unlink(inName);

	if (retErr == -1) {

		return false;

	}

	return true;
}

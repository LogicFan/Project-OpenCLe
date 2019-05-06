#include "queue.hpp"

using namespace opencle;

#include <cstdlib>
#include <exception>
#include <fstream>
#include <thread>
#include <cstdlib>		// rand()
#include <unistd.h>		// usleep()

bool rdm() { return (rand() & ((1 << 20) - 1)) == 0; }

int main(int argc, char** argv)
{
    const unsigned long long CAP = 1024ULL * 1024ULL * 32ULL;
    const unsigned int ROUNDS = 3;

    // Disable buffering (so that when run in, e.g., Sublime Text, the output appears as it is written)
    std::setvbuf(stdout, nullptr, _IONBF, 0);

    std::printf("Running stability test for opencle::queue.\n");
    std::printf("Logging to 'writeLog.txt' and 'readLog.txt'. Press CTRL+C to quit.\n\n");

    std::ofstream writeLog("writeLog.txt");
    std::ofstream readLog("readLog.txt");

    opencle::queue<unsigned long long, CAP> q;

    for (unsigned int i = 0; i < ROUNDS; ++i) {
        writeLog << "---------------------------------\n" << "Test #" << i << std::endl;
        readLog << "---------------------------------\n" << "Test #" << i << std::endl;
        std::printf("Test #%d\n", i);

        std::thread writer([&]() {
            unsigned long long delayTimes = 0ULL;
            unsigned long long testSizeTimes = 0ULL;
            unsigned long long testEmptyTimes = 0ULL;

            for (unsigned long long j = 0; j < CAP; ++j) {
                if ((j & (1024 * 1024 - 1)) == 0) {
                    writeLog << "  ... iteration " << j << std::endl;
                }

                // random sleeps
                if (rdm()) {
                    ++delayTimes;
                    writeLog << "        ... Write: delayed" << std::endl;
                    usleep(2000);
                }
                // test for size approx
                if (rdm()) {
                    ++testSizeTimes;
                    writeLog << "        ... Write: size approx = " << q.size() << std::endl;
                }
                // test for empty approx
                if (rdm()) {
                    ++testEmptyTimes;
                    writeLog << "        ... Write: empty approx = " << std::boolalpha << q.empty() << std::endl;
                }

                q.push(j);
            }

            writeLog << "\n**********************" << std::endl
                << "Test #" << i << " write finished." << std::endl
                << "  delayTimes = " << delayTimes << std::endl
                << "  testSizeTimes = " << testSizeTimes << std::endl
                << "  testEmptyTimes = " << testEmptyTimes << " out of " << CAP << std::endl
                << "**********************\n" << std::endl;
        });

        std::thread reader([&]() {
            bool canLog = true;
            bool mutated;

            unsigned long long element;

            unsigned long long popEmptyTimes = 0ULL;    // For times where read is faster than write
            unsigned long long mutationTimes = 0ULL;    // For times when front is mutated
            unsigned long long delayTimes = 0ULL;
            unsigned long long testSizeTimes = 0ULL;
            unsigned long long testEmptyTimes = 0ULL;

            usleep(2000);

            for (unsigned long long j = 0; j < CAP;) {
                mutated = false;

                if (canLog && (j & (1024 * 1024 - 1)) == 0) {
                    readLog << "  ... iteration " << j << std::endl;
                    canLog = false;
                }

                // test for mutation
                if (rdm()) {
                    ++mutationTimes;
                    mutated = true;
                    q.front() = -1ULL;
                    readLog << "        ... Read: front value mutated to -1" << std::endl;
                }
                // random sleeps
                if (rdm()) {
                    ++delayTimes;
                    readLog << "        ... Read: delayed" << std::endl;
                    usleep(2000);
                }
                // test for size approx
                if (rdm()) {
                    ++testSizeTimes;
                    readLog << "        ... Read: size approx = " << q.size() << std::endl;
                }
                // test for empty approx
                if (rdm()) {
                    ++testEmptyTimes;
                    readLog << "        ... Read: empty approx = " << std::boolalpha << q.empty() << std::endl;
                }


                try {
                    q.pop(element);
                    if ((!mutated && element != j) || (mutated && element != -1ULL)) {
                        readLog << "  ERROR DETECTED: Expected to read " << j << " but found " << element << std::endl;
                        std::printf("  ERROR DETECTED: Expected to read %llu but found %llu\n", j, element);
                    } else if (mutated && element == -1ULL) {
                        readLog << "            ... Confirm: Value mutated." << std::endl;
                    }
                    ++j;
                    canLog = true;
                } catch (std::out_of_range) {
                    readLog << "        ... ... ... ... ... ... ... ... An empty pop. Size approx = " << q.size() << std::endl;
                    ++popEmptyTimes;
                }
            }

            try {
                q.pop(element);
                readLog << "  ERROR DETECTED: Expected queue to be empty" << std::endl;
                std::printf("  ERROR DETECTED: Expected queue to be empty\n");
            } catch (std::out_of_range) {
                readLog << "\n**********************" << std::endl
                    << "Test #" << i << " read finished." << std::endl
                    << "  popEmptyTimes = " << popEmptyTimes << std::endl
                    << "  delayTimes = " << delayTimes << std::endl
                    << "  testSizeTimes = " << testSizeTimes << std::endl
                    << "  testEmptyTimes = " << testEmptyTimes << " out of " << CAP << std::endl
                    << "**********************\n" << std::endl;
            }
        });

        writer.join();
        reader.join();

        q.clear();
    }

    std::printf("All Done.\n");
    return 0;
}

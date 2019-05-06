#include <functional>
#include <vector>

namespace opencle {

enum class Device_Type { ACC, ALL, CPU, GPU, DEF };

class Memory;
class Task;

/**An RAII and Single object, used for manage all OpenCL
 * devices, and assign tasks for OpenCL devices */
class AMP final {
    using Memories = std::vector<Memory>;

  public:
    /** If ref_count = 0, initialize OpenCL devices based
     * on 'type', initialize task queue, and set ref_count
     * to be 1; if ref_count > 0, then increase ref_count. */
    AMP(Device_Type type = Device_Type::ALL);

    /** Do nothing */
    AMP(AMP const &rhs);

    /** Do nothing */
    AMP(AMP &&rhs);

    /** Decrease ref_count, and check if ref_count = 0, release
     * all resources */
    ~AMP();

    /** Do nothing */
    AMP &operator=(AMP const &rhs);

    /** Do nothing */
    AMP &operator=(AMP &&rhs);

    /** Push task into task queue, and let Scheduler decide how to
     * assgin the task, when the task in finished, call the 'call_back'*/
    Memories exec(Task const &rhs,
                  std::function<void(const Memories &)> call_back);
    Memories exec(Task &&rhs, std::function<void(const Memories &)> call_back);

    /** print all necessary infomation */
    void print_info();
};

} // namespace opencle
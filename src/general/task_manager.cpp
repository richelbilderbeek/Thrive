#include "task_manager.h"

using namespace thrive;

Task::Task() { }

Task::~Task() { }

bool
Task::isReady()
{
    return true;
}

struct TaskManager::Implementation{
    size_t m_availableThreads = boost::thread::hardware_concurrency();
};

TaskManager&
TaskManager::instance() {
    static TaskManager instance;
    return instance;
}

TaskManager::TaskManager()
    : m_impl(new Implementation())
{
}

TaskManager::~TaskManager()
{
//    std::cout << "TM Stopping Threads...." << std::endl;
    for(auto thread : m_threads)
        thread->join();
//    std::cout << "TM Threads Stopped!" << std::endl;
}

size_t
TaskManager::getAvailableThreads()
{
    return m_availableThreads;
}

void
TaskManager::start()
{
    m_running = true;

    for(size_t i = 0; i < m_impl->m_availableThreads; i++)
        m_threads.push_back(new boost::thread(boost::bind(&TaskManager::worker,this)));

//    std::cout << "Added " << m_threads.size() << " threads" << std::endl;

    while (m_running)
    {
        boost::this_thread::yield();
    }
}

void
TaskManager::stop()
{
//    std::cout << "Setting Stop!" << std::endl;
    m_running = false;
}

void
TaskManager::addTask(TaskPtr t)
{
    boost::mutex mu;
    boost::lock_guard<boost::mutex> lock(mu);

    m_taskQueue.push_back(t);
}

bool
TaskManager::tryGetTask(TaskPtr& result)
{
    boost::mutex mu;
    boost::lock_guard<boost::mutex> lock(mu);

//    std::cout << "Try Task....Total of " << m_taskQueue.size() << std::endl;

    for(size_t i=0; i<m_taskQueue.size(); i++)
    {
//        std::cout << "Task Ready? " << m_taskQueue.at(i)->isReady() << std::endl;

        if( m_taskQueue.at(i)->isReady() )
        {
            result = m_taskQueue.at(i);
            m_taskQueue.erase(m_taskQueue.begin() + i);
            return true;
        }
    }

    return false;
}

void
TaskManager::worker()
{
    TaskPtr task;
    /*
    boost::mutex mu;

    {
        boost::lock_guard<boost::mutex> lock(mu);
        std::cout << "Worker " << boost::this_thread::get_id() << " started!" << std::endl;
    }
    */

    while(m_running)
    {
        if(tryGetTask(task))
        {
            task->run();
        }
        else
        {
    /*
            {
                boost::lock_guard<boost::mutex> lock(mu);
                std::cout << "Worker " << boost::this_thread::get_id() << " Yielding!" << std::endl;
            }
    */
            boost::this_thread::yield();
        }
    }

    /*
    {
        boost::lock_guard<boost::mutex> lock(mu);
        std::cout << "Worker " << boost::this_thread::get_id() << " Stoppped!" << std::endl;
    }
    */
}
#include <bits/stdc++.h>
#include <SFML/Graphics.hpp>

static int timer = 0;
static int nextProcessArrivalTime = 0;
std::map<int, Process *> mid;

enum STATE
{
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

enum TYPE_OF_PROCESS
{
    KERNEL,
    BACKGROUND,
    APPLICATIONS
};

class Process
{
public:
    int m_arrivalTime, m_burstTime, m_waitTime, m_id, m_priority;
    STATE m_state;
    sf::Color m_color;
    TYPE_OF_PROCESS m_tag;

public:
    Process(int arrivalTime, int burstTime, int id, int priority, sf::Color color, TYPE_OF_PROCESS tag = KERNEL)
        : m_arrivalTime(arrivalTime), m_burstTime(burstTime), m_waitTime(0), m_id(id), m_priority(priority), m_state(READY), m_color(color), m_tag(tag) {}
};

std::vector<Process *> takeProcessInput()
{
    int color = 0;
    std::cout << "Enter the number of processes to schedule ";
    int n;
    std::cin >> n;

    std::cout << std::endl;
    std::cout << "Enter the Process characteristics with the following format " << std::endl;
    std::cout << "<ArrivalTime> <BurstTime> <Process_ID> <Priority> <Type_of_process>" << std::endl;
    std::cout << "Type_of_process can be Kernel or system processes, background processes, or applications. " << std::endl;
    std::cout << "Enter 0 if the process is Kernel or a system process, 1 if it is background, otherwise 2. " << std::endl
              << std::endl;

    std::vector<Process *> allProcesses(n, NULL);

    Process *p;
    int t_arrivalTime, t_burstTime, t_id, t_priority, t_tag_idx;
    sf::Color t_color;
    TYPE_OF_PROCESS t_tag;

    for (int i = 0; i < n; i++)
    {
        std::cin >> t_arrivalTime >> t_burstTime >> t_id >> t_priority >> t_tag_idx;

        if (t_tag_idx == 0)
        {
            t_tag = KERNEL;
        }
        else if (t_tag_idx == 1)
        {
            t_tag = BACKGROUND;
        }
        else
        {
            t_tag = APPLICATIONS;
        }

        if (color == 0)
        {
            t_color = sf::Color::Green;
        }
        else if (color == 1)
        {
            t_color = sf::Color::Blue;
        }
        else
        {
            t_color = sf::Color::Red;
        }

        color = (color + 1) % 3;

        p = new Process(t_arrivalTime, t_burstTime, t_id, t_priority, t_color, t_tag);
        mid[t_priority] = p;
        allProcesses[i] = p;
    }

    std ::cout << std ::endl;
    return allProcesses;
}

class comparator1
{
public:
    bool operator()(Process *a, Process *b)
    {
        return a->m_priority < b->m_priority;
    }
};

class Scheduler
{
public:
    std::vector<Process *> processes;
    std::deque<Process *> background, applications;
    std::priority_queue<Process *, std::vector<Process *>, comparator1> kernel;
    std::vector<std::pair<int, int>> res;
    int idx;

    static bool comparator2(Process *a, Process *b)
    {
        return a->m_arrivalTime < b->m_arrivalTime;
    }

    void updateQueue()
    {
        while (timer >= nextProcessArrivalTime)
        {
            if (processes[idx]->m_tag == 0)
                kernel.push(processes[idx++]);
            else if (processes[idx]->m_tag == 1)
                background.push_back(processes[idx++]);
            else
                applications.push_back(processes[idx++]);

            if (idx < processes.size())
            {
                nextProcessArrivalTime = processes[idx]->m_arrivalTime;
            }
            else
            {
                nextProcessArrivalTime = INT_MAX;
            }
        }
    }

    // Priority scheduling.
    void scheduleKernel()
    {
        while (!kernel.empty() && timer < nextProcessArrivalTime)
        {
            auto e = kernel.top();
            kernel.pop();

            if (res.size() && (res.back().second == e->m_id))
            {
            }
            else
                res.push_back({timer, e->m_id});

            if ((timer + e->m_burstTime) > nextProcessArrivalTime)
            {
                e->m_burstTime -= nextProcessArrivalTime - timer;
                timer = nextProcessArrivalTime;
                kernel.push(e);
            }
            else
                timer += e->m_burstTime;
        }
    }

    // RR WITH 4ms.
    void scheduleApplications()
    {
        while (applications.size() && (timer < nextProcessArrivalTime))
        {
            auto e = applications.front();
            applications.pop_front();

            if (res.size() && (res.back().second == e->m_id))
            {
            }
            else
                res.push_back({timer, e->m_id});

            timer += std::min(4, e->m_burstTime);
            e->m_burstTime -= std::min(e->m_burstTime, 4);

            if (timer >= nextProcessArrivalTime)
            {
                updateQueue();
            }

            if (e->m_burstTime)
                applications.push_back(e);
        }
    }

    // FCFS
    void scheduleBackground()
    {
        while (background.size() && (timer < nextProcessArrivalTime))
        {
            auto e = background.front();
            background.pop_front();

            if (res.size() && (res.back().second == e->m_id))
            {
            }
            else
                res.push_back({timer, e->m_id});

            timer += e->m_burstTime;
        }
    }

    void taskDivision()
    {
        if (!kernel.empty())
        {
            scheduleKernel();
            checkTimer();
        }

        if (!applications.empty())
        {
            scheduleApplications();
            checkTimer();
        }

        if (!background.empty())
        {
            scheduleBackground();
            checkTimer();
        }
    }

    void checkTimer()
    {
        if (timer >= nextProcessArrivalTime)
        {
            updateQueue();
            taskDivision();
        }
    }

    void schedule(std::vector<Process *> &allProcesses)
    {
        idx = 0;
        if (allProcesses.size() == 0)
        {
            return;
        }

        for (auto e : allProcesses)
        {
            processes.push_back(e);
        }

        std::sort(processes.begin(), processes.end(), comparator2);

        while (idx < processes.size())
        {
            if (kernel.empty() && applications.empty() && background.empty())
            {
                nextProcessArrivalTime = processes[idx]->m_arrivalTime;

                if (timer < nextProcessArrivalTime)
                {
                    res.push_back({timer, -1});
                    timer = nextProcessArrivalTime;
                }
            }

            checkTimer();
        }

        res.push_back({timer, -1});
    }
};

void drawGanttChart(sf::RenderWindow &window, std::vector<std::pair<int, int>> &processes, float totalDuration)
{
    const float barHeight = 30.0f;
    const float padding = 10.0f;
    const float startY = 100.0f;
    const float scale = window.getSize().x / totalDuration;
    int n = processes.size(), i = 0;

    for (auto &process : processes)
    {
        Process *p = mid[process.second];
        float startX = padding + process.first * scale;
        float width;

        if (i < n - 1)
        {
            width = scale * (processes[i + 1].first - processes[i].first);
        }
        else
        {
            width = scale * (totalDuration - processes[i].first);
        }

        std::string name = std::to_string(p->m_id);

        sf::RectangleShape processBar(sf::Vector2f(width, barHeight));
        processBar.setPosition(startX, startY);
        processBar.setFillColor(p->m_color);

        window.draw(processBar);

        sf::Text text;
        text.setString(name);
        text.setFillColor(sf::Color::Black);
        text.setPosition(startX, startY + barHeight + 5.0f);

        window.draw(text);
        i++;
    }
}

int main()
{
    sf::RenderWindow window(sf::VideoMode(800, 600), "Scheduler results");

    std::vector<Process *> allProcesses = takeProcessInput();
    Scheduler scheduler;
    scheduler.schedule(allProcesses);
    std::vector<std::pair<int, int>> res = scheduler.res;

    float totalDuration = 0.0f;
    for (const auto &process : res)
    {
        float endTime = mid[process.second]->m_arrivalTime + mid[process.second]->m_burstTime;
        if (endTime > totalDuration)
            totalDuration = endTime;
    }

    while (window.isOpen())
    {
        sf::Event event;
        while (window.pollEvent(event))
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }

        window.clear(sf::Color::White);

        drawGanttChart(window, res, totalDuration);

        window.display();
    }

    return 0;
}
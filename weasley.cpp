#include <iostream>
#include <omp.h>
#include <exception>
#include <string.h>
#include <ctime>

#define SIZE_KB(kb) 16*kb
#define SIZE_MB(mb) 16*1024*mb

//#define DEBUG

struct Item {
#ifdef DEBUG
    int num;
#endif
    union {
        Item *next;
        uint8_t data[64];
    } data;
};

#ifdef DEBUG
void items_out (Item *items, const std::size_t num, Item *p_first);
#endif
void init (Item* items, const std::size_t num);
void shuffle (Item *items, const std::size_t num, Item **p_first);
void test(Item *start_p, std::size_t num);

int main (int argc, char **argv) {
    std::srand(time(0));
    Item *items;
    Item *p_first;
    std::size_t num;
    std::string arg1;
    std::size_t arg2;
    if (argc == 3) {
        try {
            arg1 = std::string(argv[1]);
            arg2 = std::stoi(argv[2]);
            if (arg1 != "k" && arg1 != "m") {
                throw std::runtime_error("Invalid first argument! k or m was expected!");      
            }
            if (arg2 <= 0) {
                throw std::runtime_error("Second argument must to be more than 0");
            }
            if (arg1 == "k") {
                num = SIZE_KB(arg2);
                items = new Item[num];
                p_first = items;
            }
            if (arg1 == "m") {
                num = SIZE_MB(arg2);
                items = new Item[num];
                p_first = items;
            }
        }
        catch (const std::exception & ex) {
            std::cerr << "Error: " << ex.what() << std::endl;
            return 1;
        }
    } else {
        std::cout << "Please input aruments first:" << std::endl <<
            "1 – k/m as num of kilobytes/megabytes" << std::endl <<
            "2 – num of kilobytes or megabytes" << std::endl;
        return 0;
    }
    std::cout << "Testing of kuRAM started:" << std::endl << std::endl;
    std::cout << "Runtime params : " << arg2;
    if (arg1 == "k") {
        std::cout << "Kb" << std::endl;
    }
    if (arg1 == "m") {
        std::cout << "Mb" << std::endl;
    }
    std::cout << "Num of elements: " << num << std::endl;

    std::cout << "Initialization... ";
    init(items, num);
    std::cout << "Done!\n";

    std::cout << "Shuffle... ";
    shuffle(items, num, &p_first);
    std::cout << "Done!\n";
    test(p_first, num);
#ifdef DEBUG
    items_out(items, num, p_first);
#endif 
    delete [] items;
    return 0;
}

void init (Item* items, const std::size_t num) {
    for (int i = 0; i < num - 1; i++) {
        items[i].data.next = &items[i+1];        
#ifdef DEBUG
        items[i].num = i;
#endif
    }
    items[num - 1].data.next = nullptr; 
#ifdef DEBUG
    items[num - 1].num = num - 1;
#endif
}

#ifdef DEBUG
void items_out (Item *items, const std::size_t num, Item *p_first) {
    for (int i = 0; i < num; i++) {
        std::cout << items[i].num << std::endl;
    }
    std::cout << std::endl;
    Item *it = p_first;
    while (it != nullptr) {
        printf("%d\tnext: %p \n",it->num, it->data.next);
        it = it->data.next;
    }
}
#endif


void shuffle (Item *items, const std::size_t num, Item **p_first) {
    for (int i = 0; i < num; i++) {
        std::size_t new_place = std::rand() % num;   
        bool found_1 = false;
        bool found_2 = false;
        if (&items[i] ==  *p_first) {
            *p_first = &items[new_place];
        } else if (&items[new_place] == *p_first) {
            *p_first = &items[i];
        }
        std::swap(items[i], items[new_place]);
        for (int j = 0; j < num && (!found_1 || !found_2); j++) {
            if (!found_1) {
                if (items[j].data.next == &items[i]) {
                    items[j].data.next = &items[new_place];
                    found_1 = true;
                    continue;
                }
            }
            if (!found_2) {
                if (items[j].data.next == &items[new_place]) {
                    items[j].data.next = &items[i];
                    found_2 = true;
                    continue;
                }
            }
        }
    }
}

void test(Item *start_p, std::size_t num) {
    std::cout << std::endl;
    std::cout << "Time test started..." << std::endl;
    Item *ptr = start_p;
    std::size_t test_num = 0;
    double time_begin = omp_get_wtime();
    while (ptr != nullptr) {
        ptr = ptr->data.next;
        test_num++;
    }
    double time_end = omp_get_wtime();
    std::cout << "Finished in " << time_end - time_begin << " seconds" << std::endl;
    std::cout << "Size test: ";
    if (test_num == num) {
        std::cout << "Successfully!" << std::endl;
        std::cout << "Results: " << (time_end - time_begin) / num << " seconds spent per 1 read" << std::endl;
    } else {
        std::cout << "Unsuccessfully! " << test_num << " elements read!" << std::endl;
    }
}
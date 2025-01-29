#ifndef CONTAINER_H
#define CONTAINER_H

/**
 * Checks if a item exists in a container
 *
 * @param c The container to check
 * @param t The item to search for
 *
 * @return true if the item exists, false otherwise
 */
template<class C, typename T>
static bool find_in(C &c, T t) {
    return std::find(std::begin(c), std::end(c), t) != std::end(c);
}

#endif // CONTAINER_H

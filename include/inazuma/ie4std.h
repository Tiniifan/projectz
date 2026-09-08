#ifndef IE4STD_H
#define IE4STD_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Allocates memory using the game's custom memory allocator
 * @param context Memory context identifier for allocation tracking
 * @param unk1 Unknown parameter, possibly allocation flags or alignment
 * @param size Size of memory to allocate in bytes
 * @param unk2 Unknown parameter, possibly allocation type or priority
 * @return int Memory address of allocated block, or 0 if allocation failed
 * @note This is the game's custom memory management system, not standard malloc
 * @warning Always check return value for 0 (allocation failure) before using
 * @warning Memory allocated with this function must be freed with allocatorFree
 */
int allocatorAlloc(int context, int unk1, unsigned int size, unsigned int unk2);

/**
 * @brief Frees memory previously allocated with allocatorAlloc
 * @param context Pointer to memory context identifier used during allocation
 * @param allocator Memory address of the block to free (returned by allocatorAlloc)
 * @param unk1 Unknown parameter, possibly deallocation flags
 * @return int Status code indicating success/failure of deallocation
 * @note Must be called to properly release memory allocated with allocatorAlloc
 * @warning Only call this function with memory addresses returned by allocatorAlloc
 * @warning Do not use the memory address after calling this function
 * @warning The context parameter must match the one used during allocation
 */
int allocatorFree(int *context, unsigned int allocator, int unk1);

/**
 * @brief Draws the next value from the game's global xorshift128 RNG
 * @param modulo Upper bound of the returned value, or 0 for the raw 32 bit state
 * @return int The new RNG state, reduced modulo `modulo` when it is non zero
 * @note This is the RNG the save cipher draws its seed from
 * @warning Every call advances the global state, which is shared with the rest
 * of the game
 */
int getRandomNumber(unsigned int modulo);

/**
 * @brief Acquires the game's global allocator lock
 * @note The lock is re-entrant: the game itself nests acquisitions, for example
 * around the save file encryption
 * @warning Every call must be paired with an unlockAllocator
 */
void lockAllocator(void);

/**
 * @brief Releases the game's global allocator lock
 * @warning Only call this after a matching lockAllocator
 */
void unlockAllocator(void);

#ifdef __cplusplus
}
#endif

#endif // IE4STD_H
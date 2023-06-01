// Copyright 2023 Brendan Han

/**
 * A multithreaded program to factorize numbers and print information
 * about the factors.
 *
 */

#include <bits/stdc++.h>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <cmath>
#include <utility>
#include <algorithm>
#include "hw5.h"

BigInt factorize(const BigInt& num);

/** A data parallel method to count the factors for a given list of
 * values using multiple threads.
 *
 * \param[in] numList The list of numbers whose factor counts are to
 * be computed.
 * 
 * \param[in] answer The string that outputs whether or not the number is prime
 *
 */
void threadMain(const BigIntVec& numList, std::string& answer, int thr) {
    answer = "";

    // Checking first to see if the number is prime
    if (isPrime(numList[thr])) {
        answer = std::to_string(numList[thr]) + ": Is already prime.";
    } else {
        BigInt smallestDiv = factorize(numList[thr]);
        BigInt otherFactor = numList[thr] / smallestDiv;

        // Formulating a response to be store in the vector
        answer =  std::to_string(numList[thr]) + " = " +
        std::to_string(smallestDiv) + " (prime) * " +
        std::to_string(otherFactor) +
        (isPrime(otherFactor) ? " (prime)" : "");
    }
}

/** A data parallel method to count the factors for a given list of
 * values using multiple threads.
 *
 * \param[in] numVec The list of numbers whose factor counts are to
 * be computed.
 *
 * \return The threaded responses
 */
StrVec factorize(const BigIntVec& numVec) {
    // First allocate the return vector
    int thrCount = numVec.size();
    StrVec factCounts(numVec.size());
    
    // Compute the range of values each thread should iterate over.
    // const int count = (numVec.size() + thrCount - 1) / thrCount;
    // Now spin-up threads to do the work.
    std::vector<std::thread> thrList(thrCount);
    for (int thr = 0; (thr < thrCount); thr++) {
        // const size_t start = thr * count;
        // const size_t end = std::min(numVec.size(), start + count);

        thrList[thr] = (std::thread(threadMain, std::ref(numVec),
        std::ref(factCounts[thr]), thr));
    }

    // Wait for all of the threads to finish.
    for (auto& t : thrList) {
        t.join();
    }

    // Return the result back
    return factCounts;
}

/** A helper method to the get2ndMax method
 *
 * \param[in] numList The list of numbers from where the maximum and the
 * second maximum value is to be computed.
 *
 * \param[in] thrMax The maximum value outputed by the method
 * 
 * \param[in] the2ndMax The second maximum value outputed by the method
 *
 * \param[in] start The starting index of the array
 * 
 * \param[in] end The ending index of the array
 *
*/
void threadMain2(const BigIntVec& numList, BigInt& thrMax,
BigInt& thr2ndMax, const int start, const int end) {
    BigInt larVal = numList[0], secLarVal = numList[1];

    //--------------------------------------------------------------
    //  Implement solution to determine 2nd largest value from
    //  the input stream is
    //--------------------------------------------------------------
    if (larVal < secLarVal) {
        // swap max and secMax to get them in the
        // right order
        std::swap(larVal, secLarVal);
    }

    // repeatedly read values
    for (int i = start; i < end; i++) {
        BigInt val = numList[i];

        if (val > larVal) {
            secLarVal = larVal;
            larVal = val;
        } else if (val > secLarVal && val < larVal) { 
            secLarVal = val; 
        }
    }

    // Assigning values to thread
    thrMax = larVal;
    thr2ndMax = secLarVal;
}

/** A data parallel method to compute the 2nd maximum value in a given
 * list of values using multiple threads.  This method uses
 * "reduction" approach for computing the 2nd-maximum-value using
 * multiple threads.
 *
 * \param[in] numList The list of numbers from where the maximum value
 * is to be computed.
 *
 * \param[in] thrCount The number of threads to be used. The starter
 * code ignores this parameter.
 *
 * \return The 2nd maximum value in the given numList.
 * numList.
 */
BigInt get2ndMax(const BigIntVec& numList, const int thrCount) {
    // The 2nd maximum logic was already done in class (see Exercise
    // #1). Suitably use the logic here.

    // Tip: Each thread should compute the max and 2nd-max. Then the
    // main thread should use these values to compute the overall
    // 2nd-max

    BigIntVec thrMax(thrCount);
    BigIntVec thr2ndMax(thrCount);

    // Compute the range of values each thread should iterate over.
    const int size = numList.size();
    const int count = (numList.size() / thrCount) + 1;
    // Now spin-up threads to do the work.
    std::vector<std::thread> thrList(thrCount);
    for (int start = 0, thr = 0; (thr < thrCount); thr++, start += count) {
        int end = std::min(size, (start + count));

        thrList[thr] = (std::thread(threadMain2, std::ref(numList),
        std::ref(thrMax[thr]), std::ref(thr2ndMax[thr]), start, end));
    }

    // Wait for all of the threads to finish.
    for (auto& t : thrList) {
        t.join();
    }

    // Getting two max values from a vector of just max values and a vector of
    // second max values
    BigInt firstMax = *std::max_element(thrMax.begin(), thrMax.end());
    BigInt secondMax = *std::max_element(thr2ndMax.begin(), thr2ndMax.end());

    // returning the minimum value between the two
    return std::min(firstMax, secondMax);
}

/**
 * Test resource utilities
 **/

#ifndef KAUAI_TEST_RESOURCES_H
#define KAUAI_TEST_RESOURCES_H

#include "util.h"

/**
 * @brief Set the FNI to the test resources directory.
 */
void GetTestResourcePath(PFNI pfniTestResourcePath);

/**
 * @brief Find a test resource file by name. Set the given FNI to the path if found.
 *
 * @param pfni FNI to set to the path to the resource
 * @param pszName Resource filename
 * @return fTrue if the resource was found
 */
bool FFindTestResource(PFNI pfni, PCSZ pszName);

/**
 * @brief Set the FNI to the test resource. Fails the test if not found.
 *
 * @param pfni FNI to set to the path to the resource
 * @param pszName Resource filename
 **/
void GetTestResource(PFNI pfni, PCSZ pszName);

#endif // KAUAI_TEST_RESOURCES_H
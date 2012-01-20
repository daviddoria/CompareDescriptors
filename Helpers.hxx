/*=========================================================================
 *
 *  Copyright David Doria 2011 daviddoria@gmail.com
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *         http://www.apache.org/licenses/LICENSE-2.0.txt
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *=========================================================================*/

#include <cmath>

namespace Helpers
{

template <typename T>
float ArrayDifference(T* const array1, T* const array2, const unsigned int length)
{
  float totalDifference = 0.0f;
  for(unsigned int i = 0; i < length; ++i)
    {
    totalDifference += fabs(array1[i] - array2[i]);
    }
  return totalDifference;
}

} // end namespace

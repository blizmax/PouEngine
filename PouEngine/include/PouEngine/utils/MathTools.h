#ifndef MATHTOOLS_H
#define MATHTOOLS_H

#include "PouEngine/utils/SimpleNode.h"

namespace pou::MathTools
{

struct Box
{
    glm::vec2 size;
    glm::vec2 center;
};

bool detectSegmentCollision(glm::vec2 seg1, glm::vec2 seg2);

bool detectBoxCollision(const Box &box1, const Box &box2,
                        const SimpleNode *node1 = nullptr, const SimpleNode *node2 = nullptr);


int intLog2(int v);


float computeWantedRotation(float startingRotation, glm::vec2 position);

}

bool uint32less(uint32_t lhs, uint32_t rhs);
bool uint32leq(uint32_t lhs, uint32_t rhs);


#endif // MATHTOOLS_H

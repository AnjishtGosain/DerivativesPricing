#include "Tree.h"

using namespace std;

models::Tree::Tree(const std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>>& treeNodes, 
	const int& nTimeSteps, 
	const double& timeToExpiry)
{
	setTreeNodes(treeNodes);
	setNTimeSteps(nTimeSteps);
	setTimeToExpiry(timeToExpiry);
}

void models::Tree::setTimeToExpiry(const double& value)
{
	if (value < -0.00000001)
		throw invalid_argument("The time to expiry for the tree must be greater than 0.");
	m_timeToExpiry = value;
}

void models::Tree::setNTimeSteps(const int& value)
{
	if (value < -0.00000001)
		throw invalid_argument("The number of time steps for the tree must be greater than 0.");
	m_nTimeSteps = value;
}


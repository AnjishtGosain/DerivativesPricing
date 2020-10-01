#include "TreeNode.h"

using namespace std;

// The first constructor is used for the nodes at option expiry. Cannot setup the second constructor with default values for the last
// three parameters because to do so would require the vector inputs to be const, which would cause the copy constructor to be called
// when std::move is used in the implementation.

// Setters for the class members have been intentially left out.

models::TreeNode::TreeNode(const double& value)
{
	m_value = value;
}

models::TreeNode::TreeNode(const double& value, const std::shared_ptr<std::vector<int>>& forwardValuesIndex, 
	const std::shared_ptr<std::vector<double>>& forwardProbabilities)
{
	// Input Validation
	if (forwardProbabilities->size() != forwardValuesIndex->size())
		throw invalid_argument("The forward probabilities and values must have the same size.");

	m_value = value;
	m_forwardValuesIndex = move(forwardValuesIndex); // move avoids copying forwardProbabilities
	m_forwardProbabilities = move(forwardProbabilities); // move avoids copying forwardProbabilities
	m_nForwardValues = m_forwardProbabilities->size();
}

void models::TreeNode::setValue(const double& value)
{
	m_value = value; 
}


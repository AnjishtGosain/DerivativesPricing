#ifndef __TREE_H__ 
#define __TREE_H__

#include <iostream>
#include <vector>
#include <memory>
#include "TreeNode.h"

using namespace std;

namespace models
{
	class Tree
	{
	public:
		Tree(const std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>>& treeNodes, const int& nTimeSteps, 
			const double& timeToExpiry);
		Tree() = default;
		~Tree() = default;

		const std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>> getTreeNodes() const { return m_treeNodes; }
		const int& getNTimesSteps() const { return m_nTimeSteps; }
		const double& getTimeToExpiry() const { return m_timeToExpiry; }

		Tree& operator = (Tree const&) = delete;
		Tree(Tree const&) = delete;

	private:
		std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>> m_treeNodes; // the actual tree
		int m_nTimeSteps; // the number of time steps in the tree
		double m_timeToExpiry; // the time to maturity for the tree

		// Setters
		void setTreeNodes(const std::shared_ptr<std::vector<std::shared_ptr<std::vector<std::shared_ptr<models::TreeNode>>>>>& value) 
		{ m_treeNodes = value; }
		void setNTimeSteps(const int& value);
		void setTimeToExpiry(const double& value);
	};
}

#endif // !__TREE_H__ 

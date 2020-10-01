#ifndef __TREENODE_H__ 
#define __TREENODE_H__

#include <iostream>
#include <vector>
#include <memory>

using namespace std;

namespace models
{
	class TreeNode
	{
	public:
		TreeNode(const double& value); 
		TreeNode(const double& value, const std::shared_ptr<std::vector<int>>& forwardValuesIndex, 
			const std::shared_ptr<std::vector<double>>& forwardProbabilities);
		TreeNode() = default;
		~TreeNode() = default;

		const std::shared_ptr<std::vector<int>> getForwardValuesIndex() const { return m_forwardValuesIndex; }
		const int& getNForwardValues() const { return m_nForwardValues; }
		const std::shared_ptr<std::vector<double>> getForwardProbabilities() const { return m_forwardProbabilities; }
		const double& getValue() const { return m_value; }

		void setForwardValuesIndex(const std::shared_ptr<std::vector<int>>& value) { m_forwardValuesIndex = value; m_nForwardValues = value->size(); }
		void setForwardProbabilities(const std::shared_ptr<std::vector<double>>& value) { m_forwardProbabilities = value; }

		void setValue(const double& value); // for deduction of dividend

		TreeNode& operator = (TreeNode const&) = delete;
		TreeNode(TreeNode const&) = delete;
	private:
		double m_value;
		int m_nForwardValues; // the number of nodes at the next time step which can be hit from the current node
		std::shared_ptr<std::vector<int>> m_forwardValuesIndex; // the index of the nodes at the next time step
		std::shared_ptr<std::vector<double>> m_forwardProbabilities; // the probability of hitting each of the nodes in _forwardValuesIndex
	};
}

#endif // !__TREENODE_H__ 

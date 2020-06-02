#pragma once


namespace Storm
{
	class IRigidBody;

	class GraphicRigidBody
	{
	public:
		GraphicRigidBody(const std::vector<Storm::Vector3> &vertices, const std::vector<Storm::Vector3> &normals);

	public:
		void setRbParent(const std::shared_ptr<Storm::IRigidBody> &boundRbParent);
		std::shared_ptr<Storm::IRigidBody> getRbParent();
		const std::shared_ptr<Storm::IRigidBody>& getRbParent() const;

	private:
		std::shared_ptr<Storm::IRigidBody> _boundParentRb;
		std::vector<DirectX::XMFLOAT3> _vertices;
		std::vector<DirectX::XMFLOAT3> _normals;
	};
}

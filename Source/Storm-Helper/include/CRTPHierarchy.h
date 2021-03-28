#pragma once


namespace Storm
{
	template<class Child, class Other>
	struct NotEqual
	{
	public:
		friend constexpr bool operator!=(const Child &right, const Other &left)
		{
			return !(right == left);
		}
	};

	template<class Child, class Other>
	struct InvertNotEqual
	{
	public:
		friend constexpr bool operator!=(const Other &right, const Child& left)
		{
			return !(left == right);
		}
	};

	template<class Child>
	struct InvertNotEqual<Child, Child>
	{};

	template<class Child, class Other>
	struct InvertEqual
	{
	public:
		friend constexpr bool operator==(const Other &right, const Child& left)
		{
			return left == right;
		}
	};

	template<class Child>
	struct InvertEqual<Child, Child>
	{};

	template<class Child, class Other>
	struct Hierarchisable
	{
	public:
		friend constexpr bool operator>=(const Child& right, const Other &left)
		{
			return !(right < left);
		}

		friend constexpr bool operator<=(const Child& right, const Other& left)
		{
			return (right < left) || (right == left);
		}

		friend constexpr bool operator>(const Child& right, const Other& left)
		{
			return !(right <= left);
		}
	};


	template<class Child, class Other>
	struct InvertHierarchisable
	{
	public:
		friend constexpr bool operator>=(const Other &right, const Child& left)
		{
			return left <= right;
		}

		friend constexpr bool operator<=(const Other& right, const Child& left)
		{
			return left >= right;
		}

		friend constexpr bool operator>(const Other& right, const Child& left)
		{
			return left < right;
		}

		friend constexpr bool operator<(const Other& right, const Child& left)
		{
			return left > right;
		}
	};

	template<class Child>
	struct InvertHierarchisable<Child, Child>
	{};


	template<class Child, class Other = Child>
	struct FullEquatable :
		private NotEqual<Child, Other>,
		private InvertNotEqual<Child, Other>,
		private InvertEqual<Child, Other>
	{};

	template<class Child, class Other = Child>
	struct FullHierarchisable :
		private FullEquatable<Child, Other>,
		private Hierarchisable<Child, Other>,
		private InvertHierarchisable<Child, Other>
	{};
}

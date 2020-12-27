#pragma once


namespace Storm
{
	template<class ThreadType, class ... Others>
	void join(ThreadType &th, Others &... other)
	{
		Storm::join(th);
		Storm::join(other...);
	}

	template<class ThreadType>
	void join(ThreadType &th)
	{
		if (th.joinable())
		{
			th.join();
		}
	}

	template<class ThreadType, class ... Others>
	void detach(ThreadType &th, Others &... other)
	{
		Storm::detach(th);
		Storm::detach(other...);
	}

	template<class ThreadType>
	void detach(ThreadType &th)
	{
		if (th.joinable())
		{
			th.detach();
		}
	}
}

#pragma once


namespace Storm
{
	template<class ThreadType>
	void join(ThreadType &th)
	{
		if (th.joinable())
		{
			th.join();
		}
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

#pragma once

class AbstractInspector
{
protected:
    AbstractInspector() = default;
    ~AbstractInspector() = default;

    AbstractInspector(AbstractInspector const&) = default;
    AbstractInspector& operator=(AbstractInspector const&) = default;

    AbstractInspector(AbstractInspector&&) = default;
    AbstractInspector& operator=(AbstractInspector&&) = default;

	virtual bool isInspectable() const
	{
		return true;
	}
};

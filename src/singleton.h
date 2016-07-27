

#pragma once


/**
 * A template base-class implementing the common Singleton design-pattern.
 */
template <class T> class Singleton
{
public:
    /**
     * Gain access to the singleton-instance.
     */
    static T* instance()
    {
        if (!m_instance)
            m_instance = new T;

        return m_instance;
    };

    /**
     * Destroy the given singleton-instance, if it has been created.
     */
    static void destroy_instance()
    {
        if (m_instance)
        {
            delete m_instance;
            m_instance = NULL;
        }
    };

private:

    /**
     * The one instance of our object.
     */
    static T* m_instance;
};

template <class T> T* Singleton<T>::m_instance = NULL;

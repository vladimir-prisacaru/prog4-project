#pragma once

#include "GameObject.h"

namespace dae
{
    class ThrashTheCacheDemo : public Component
    {
        public:

        ThrashTheCacheDemo(GameObject* owner) : Component(owner) { }
        virtual ~ThrashTheCacheDemo() = default;

        void Update(float) override;

        private:

        struct transform
        {
            float matrix[16] {
                1, 0, 0, 0,
                0, 1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
            };
        };

        class gameobject1
        {
            public:

            transform local { };
            int id { };
        };

        class gameobject2
        {
            public:

            ~gameobject2() { delete local; }

            transform* local { new transform() };
            int id { };
        };

        std::vector<double> m_Strides1;
        std::vector<double> m_Results1;

        std::vector<double> m_Strides2;
        std::vector<double> m_Results2;

        std::vector<double> m_Strides3;
        std::vector<double> m_Results3;

        int m_SampleSize { 500'000 };
        int m_NumSamples { 10 };

        void RunBenchmark1(int size);
        void RunBenchmark2(int size);
        void RunBenchmark3(int size);

        static double AverageWithoutOutliers(const std::vector<double>& samples);
    };
}
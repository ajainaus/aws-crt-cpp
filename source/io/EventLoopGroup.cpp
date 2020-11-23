/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */
#include <aws/crt/io/EventLoopGroup.h>

namespace Aws
{
    namespace Crt
    {
        namespace Io
        {
            EventLoopGroup::EventLoopGroup(uint16_t threadCount, Allocator *allocator) noexcept
                : EventLoopGroup(nullptr, threadCount, allocator)
            {
            }

            EventLoopGroup::EventLoopGroup(
                const EventLoopGroupShutdownOptions *shutdownOptions,
                uint16_t threadCount,
                Allocator *allocator) noexcept
                : m_eventLoopGroup(nullptr), m_lastError(AWS_ERROR_SUCCESS)
            {
                if (shutdownOptions)
                {
                    auto wrappedData = Crt::New<EventLoopGroupShutdownOptionsWrappedData>(allocator);
                    wrappedData->fn = shutdownOptions->callback;
                    wrappedData->args = shutdownOptions->userData;
                    wrappedData->allocator = allocator;

                    aws_shutdown_callback_options options;
                    options.shutdown_callback_fn = EventLoopGroup::EventLoopGroupShutdownCallback;
                    options.shutdown_callback_user_data = static_cast<void *>(wrappedData);
                    m_eventLoopGroup = aws_event_loop_group_new_default(allocator, threadCount, &options);
                }
                else
                {
                    m_eventLoopGroup = aws_event_loop_group_new_default(allocator, threadCount, NULL);
                }

                if (m_eventLoopGroup == nullptr)
                {
                    m_lastError = aws_last_error();
                }
            }

            EventLoopGroup::~EventLoopGroup() { aws_event_loop_group_release(m_eventLoopGroup); }

            EventLoopGroup::EventLoopGroup(EventLoopGroup &&toMove) noexcept
                : m_eventLoopGroup(toMove.m_eventLoopGroup), m_lastError(toMove.m_lastError)
            {
                toMove.m_lastError = AWS_ERROR_UNKNOWN;
                toMove.m_eventLoopGroup = nullptr;
            }

            EventLoopGroup &EventLoopGroup::operator=(EventLoopGroup &&toMove) noexcept
            {
                m_eventLoopGroup = toMove.m_eventLoopGroup;
                m_lastError = toMove.m_lastError;
                toMove.m_lastError = AWS_ERROR_UNKNOWN;
                toMove.m_eventLoopGroup = nullptr;

                return *this;
            }

            int EventLoopGroup::LastError() const { return m_lastError; }

            EventLoopGroup::operator bool() const { return m_lastError == AWS_ERROR_SUCCESS; }

            aws_event_loop_group *EventLoopGroup::GetUnderlyingHandle() noexcept
            {
                if (*this)
                {
                    return m_eventLoopGroup;
                }

                return nullptr;
            }

        } // namespace Io

    } // namespace Crt
} // namespace Aws

'use client'

interface ConnectionStatusProps {
  connected: boolean
}

export default function ConnectionStatus({ connected }: ConnectionStatusProps) {
  return (
    <div style={{ marginTop: '0.5rem' }}>
      <span className={`status ${connected ? 'connected' : 'disconnected'}`}>
        {connected ? '● Connected' : '● Disconnected'}
      </span>
    </div>
  )
}


'use client'

interface ConnectionStatusProps {
  connected: boolean
}

export default function ConnectionStatus({ connected }: ConnectionStatusProps) {
  return (
    <div className={`status ${connected ? 'connected' : 'disconnected'}`}>
      {connected ? 'Connected' : 'Disconnected'}
    </div>
  )
}

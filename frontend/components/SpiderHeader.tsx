'use client'

interface SpiderHeaderProps {
  connected: boolean
}

export default function SpiderHeader({ connected }: SpiderHeaderProps) {
  return (
    <header className="spider-header">
      <div className="header-left">
        <div className="header-logo">🕷️</div>
        <h2 className="header-title glitch-text" data-text="SPIDER-TRADE">SPIDER-TRADE</h2>
        <div className="header-status">
          <span className="status-dot" style={{ background: connected ? 'var(--primary)' : 'var(--red)', boxShadow: connected ? '0 0 5px var(--primary)' : 'none' }}></span>
          <span className="status-text">{connected ? 'SYSTEM ONLINE' : 'OFFLINE'}</span>
        </div>
      </div>
      <div className="header-right">
        <button className="header-button" title="Notifications">🔔</button>
        <button className="header-button" title="Settings">⚙️</button>
      </div>
    </header>
  )
}


'use client'

interface SpiderSidebarProps {
  activeTab: string
  onTabChange: (tab: string) => void
  latency?: number
}

export default function SpiderSidebar({ activeTab, onTabChange, latency = 0 }: SpiderSidebarProps) {
  const tabs = [
    { id: 'dashboard', label: 'Dashboard', icon: '📊' },
    { id: 'markets', label: 'Markets', icon: '📈' },
    { id: 'portfolio', label: 'Portfolio', icon: '💼' },
    { id: 'analytics', label: 'Analytics', icon: '📉' },
    { id: 'history', label: 'History', icon: '🕐' },
  ]

  return (
    <aside className="spider-sidebar">
      <div className="sidebar-content">
        {/* User Profile */}
        <div className="sidebar-user">
          <div className="sidebar-avatar">M</div>
          <div className="sidebar-user-info">
            <h3 className="sidebar-username">MILES M.</h3>
            <p className="sidebar-user-title">PRO TRADER</p>
          </div>
        </div>

        {/* Navigation */}
        <nav className="sidebar-nav">
          {tabs.map(tab => (
            <button
              key={tab.id}
              onClick={() => onTabChange(tab.id)}
              className={`sidebar-nav-item ${activeTab === tab.id ? 'active' : ''}`}
            >
              <span className="sidebar-nav-icon">{tab.icon}</span>
              <span className="sidebar-nav-label">{tab.label}</span>
            </button>
          ))}
        </nav>
      </div>

      {/* Latency Indicator */}
      <div className="sidebar-footer">
        <div className="latency-card">
          <div className="latency-header">
            <span className="latency-label">LATENCY</span>
            <span className="latency-value">{latency > 0 ? `${latency}ms` : '--'}</span>
          </div>
          <div className="latency-bar">
            <div 
              className="latency-bar-fill" 
              style={{ 
                width: `${Math.min(100, (latency / 100) * 100)}%`,
                backgroundColor: latency < 50 ? '#10b981' : latency < 100 ? '#f59e0b' : '#ef4444'
              }}
            ></div>
          </div>
        </div>
      </div>
    </aside>
  )
}

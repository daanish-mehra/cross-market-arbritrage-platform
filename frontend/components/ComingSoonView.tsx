'use client'

interface ComingSoonViewProps {
  title: string
  subtitle?: string
  icon?: string
}

export default function ComingSoonView({ title, subtitle, icon = '🚀' }: ComingSoonViewProps) {
  return (
    <>
      <div className="page-header">
        <div>
          <h1 className="page-title">{title}</h1>
          <p className="page-subtitle">{subtitle || 'COMING SOON // UNDER DEVELOPMENT'}</p>
        </div>
      </div>

      <div style={{ padding: '32px', display: 'flex', justifyContent: 'center', alignItems: 'center', minHeight: '400px' }}>
        <div className="coming-soon-card">
          <div className="coming-soon-icon">{icon}</div>
          <div className="coming-soon-title">{title}</div>
          <div className="coming-soon-subtitle">
            This feature is currently under development.
            <br />
            Check back soon for updates!
          </div>
        </div>
      </div>
    </>
  )
}

